<?php

// MatchController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\TournamentMatch;
use App\Models\Tournament;
use App\Models\Participant;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\DB;
use Illuminate\Foundation\Auth\Access\AuthorizesRequests;

class MatchController extends Controller
{
    use AuthorizesRequests;
    /**
     * Detail zápasu
     */
    public function show(TournamentMatch $match)
    {
        $match->load([
            'tournament',
            'participant1.participant',
            'participant2.participant'
        ]);

        return view('matches.show', compact('match'));
    }

    /**
     * Formulár na zadanie výsledku zápasu
     */
    public function edit(TournamentMatch $match)
    {
        $this->authorize('update', $match->tournament);

        $match->load(['participant1.participant', 'participant2.participant']);

        return view('matches.edit', compact('match'));
    }

    /**
     * Ulož výsledok zápasu
     */
    public function update(Request $request, TournamentMatch $match)
    {
        $this->authorize('update', $match->tournament);

        $validated = $request->validate([
            'score1' => 'required|integer|min:0',
            'score2' => 'required|integer|min:0',
        ]);

        // Urči víťaza
        $winnerId = null;
        if ($validated['score1'] > $validated['score2']) {
            $winnerId = $match->participant1_id;
        } elseif ($validated['score2'] > $validated['score1']) {
            $winnerId = $match->participant2_id;
        }
        // Remíza -> winner_id = null

        $match->update([
            'score1' => $validated['score1'],
            'score2' => $validated['score2'],
            'winner_id' => $winnerId,
            'finished' => true,
        ]);

        if ($winnerId) {
            $this->advanceWinner($match, $winnerId);
        }

        return redirect()
            ->route('tournaments.show#bracket', $match->tournament)
            ->with('success', 'Match result has been saved.');
    }

    /**
     * To isté ale AJAX
     */
    public function quickUpdate(Request $request, TournamentMatch $match)
    {
        $this->authorize('update', $match->tournament);

        $validated = $request->validate([
            'score1' => 'required|integer|min:0',
            'score2' => 'required|integer|min:0',
        ]);

        $winnerId = null;
        if ($validated['score1'] > $validated['score2']) {
            $winnerId = $match->participant1_id;
        } elseif ($validated['score2'] > $validated['score1']) {
            $winnerId = $match->participant2_id;
        }

        $match->update([
            'score1' => $validated['score1'],
            'score2' => $validated['score2'],
            'winner_id' => $winnerId,
            'finished' => true,
        ]);

        if ($winnerId) {
            $this->advanceWinner($match, $winnerId);
        }

        return response()->json([
            'success' => true,
            'message' => 'Result has been saved.',
            'winner_id' => $winnerId,
        ]);
    }

    /**
     * Resetuje výsledok zápasu - len správca
     */
    public function reset(TournamentMatch $match)
    {
        $this->authorize('update', $match->tournament);

        $match->update([
            'score1' => null,
            'score2' => null,
            'winner_id' => null,
            'finished' => false,
        ]);

        // TODO: Odobrať účastníka z nasledujúceho zápasu ???

        return back()->with('success', 'Match result has been reset.');
    }

    /**
     * Vymaž zápas -> správca
     */
    public function destroy(TournamentMatch $match)
    {
        $this->authorize('delete', $match->tournament);

        if ($match->finished) {
            return back()->with('error', 'Finished match cannot be deleted.');
        }

        $match->delete();

        return back()->with('success', 'Match has been deleted.');
    }

    /**
     * Posuň víťaza do ďalšieho kola
     *
     * @param TournamentMatch $match
     * @param int $winnerId
     */
    protected function advanceWinner(TournamentMatch $match, int $winnerId)
    {
        $allMatches = $match->tournament->matches()->orderBy('index')->get();

        $currentIndex = $match->index;

        $totalMatches = $allMatches->count();
        $firstRoundMatches = ($totalMatches + 1) / 2;

        if ($currentIndex === $totalMatches - 1) { // finále?
            return;
        }

        $nextMatchIndex = $this->calculateNextMatchIndex($currentIndex, $firstRoundMatches);

        $nextMatch = $allMatches->firstWhere('index', $nextMatchIndex);

        if (!$nextMatch) {
            return;
        }

        if ($this->isEvenMatchInRound($currentIndex, $firstRoundMatches)) {
            $nextMatch->update(['participant1_id' => $winnerId]);
        } else {
            $nextMatch->update(['participant2_id' => $winnerId]);
        }
    }

    /**
     * Index nasledujúceho zápasu
     *
     * @param int $currentIndex
     * @param int $firstRoundMatches
     * @return int
     */
    protected function calculateNextMatchIndex(int $currentIndex, int $firstRoundMatches): int
    {
        $matchesBeforeCurrent = 0;
        $round = 1;
        $matchesInRound = $firstRoundMatches;

        while ($matchesBeforeCurrent + $matchesInRound <= $currentIndex) {
            $matchesBeforeCurrent += $matchesInRound;
            $matchesInRound = $matchesInRound / 2;
            $round++;
        }

        $indexInRound = $currentIndex - $matchesBeforeCurrent;

        $nextRoundMatchIndex = floor($indexInRound / 2);

        $matchesBeforeNextRound = $matchesBeforeCurrent + $matchesInRound;

        return $matchesBeforeNextRound + $nextRoundMatchIndex;
    }

    /**
     * Je zápas na párnej pozícii v kole?
     *
     * @param int $currentIndex
     * @param int $firstRoundMatches
     * @return bool
     */
    protected function isEvenMatchInRound(int $currentIndex, int $firstRoundMatches): bool
    {
        $matchesBeforeCurrent = 0;
        $matchesInRound = $firstRoundMatches;

        while ($matchesBeforeCurrent + $matchesInRound <= $currentIndex) {
            $matchesBeforeCurrent += $matchesInRound;
            $matchesInRound = $matchesInRound / 2;
        }

        $indexInRound = $currentIndex - $matchesBeforeCurrent;

        return $indexInRound % 2 === 0;
    }

    /**
     * Zápasy turnaja vo forme zoznamu
     * Prístup: všetci
     */
    public function tournamentMatches(Tournament $tournament)
    {
        $matches = $tournament->matches()
            ->with(['participant1.participant', 'participant2.participant'])
            ->orderBy('index')
            ->get();

        $matchesByRound = $this->groupMatchesByRound($matches);

        return view('matches.tournament-matches', compact('tournament', 'matchesByRound'));
    }

    /**
     * Zoskup podľa kola
     *
     * @param Collection $matches
     * @return array
     */
    protected function groupMatchesByRound($matches)
    {
        if ($matches->isEmpty()) {
            return [];
        }

        $totalMatches = $matches->count();
        $firstRoundMatches = ($totalMatches + 1) / 2;

        $rounds = [];
        $currentRound = 1;
        $matchesInRound = $firstRoundMatches;
        $startIndex = 0;

        while ($startIndex < $totalMatches) {
            $roundMatches = $matches->slice($startIndex, $matchesInRound)->values();

            $roundName = $this->getRoundName($currentRound, $totalMatches);

            $rounds[$roundName] = $roundMatches;

            $startIndex += $matchesInRound;
            $matchesInRound = $matchesInRound / 2;
            $currentRound++;
        }

        return $rounds;
    }

    /**
     * Názov kola
     *
     * @param int $round
     * @param int $totalMatches
     * @return string
     */
    protected function getRoundName(int $round, int $totalMatches): string
    {
        $totalRounds = log(($totalMatches + 1), 2);
        $roundsFromEnd = $totalRounds - $round + 1;

        if ($roundsFromEnd === 1) {
            return 'Finále';
        } elseif ($roundsFromEnd === 2) {
            return 'Semifinále';
        } elseif ($roundsFromEnd === 3) {
            return 'Štvrťfinále';
        } elseif ($roundsFromEnd === 4) {
            return 'Osemfinále';
        } else {
            return $round . '. kolo';
        }
    }

    /**
     * Štatistiky zápasu
     */
    public function statistics(TournamentMatch $match)
    {
        $match->load([
            'tournament',
            'participant1.participant',
            'participant2.participant'
        ]);

        $stats = [
            'total_score' => ($match->score1 ?? 0) + ($match->score2 ?? 0),
            'score_difference' => abs(($match->score1 ?? 0) - ($match->score2 ?? 0)),
        ];

        return view('matches.statistics', compact('match', 'stats'));
    }
}
