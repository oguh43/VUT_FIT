<?php

// TournamentController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\Tournament;
use App\Models\Participant;
use App\Models\TournamentMatch;
use App\Models\User;
use App\Models\Team;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\DB;
use Illuminate\Foundation\Auth\Access\AuthorizesRequests;

class TournamentController extends Controller
{
    use AuthorizesRequests;
    /**
     * Zoznam všetkých turnajov
     */
    public function index(Request $request)
    {
        $query = Tournament::with(['creator', 'participants']);

        if ($request->has('approved')) {
            $query->where('approved', $request->approved);
        }

        if ($request->has('type')) {
            $query->where('type', $request->type);
        }

        if ($request->has('search')) {
            $query->where('name', 'like', '%' . $request->search . '%');
        }

        $tournaments = $query->orderBy('date', 'desc')->paginate(12);

        return view('tournaments.index', compact('tournaments'));
    }

    /**
     * Detail turnaja -> pavúk a štatistiky
     */
    public function show(Tournament $tournament)
    {
        $tournament->load([
            'participants.participant',
            'matches',
            'creator'
        ]);

        $bracket = $this->generateBracket($tournament);

        $stats = [
            'total_participants' => $tournament->participants()->where('approved', true)->count(),
            'pending_participants' => $tournament->participants()->where('approved', false)->count(),
            'total_matches' => $tournament->matches()->count(),
            'completed_matches' => $tournament->matches()->where('finished', true)->count(),
        ];

        $userTeams = null;
        if (Auth::check() && $tournament->type === 'team') {
            $userTeams = Auth::user()->createdTeams;
        }

        $pending = $tournament->participants()->with('participant')->where('approved', false)->get();
        $approved = $tournament->participants()->with('participant')->where('approved', true)->get();

        $userSchedule = null;
        if (Auth::check()) {
            $userSchedule = $this->getUserSchedule($tournament, Auth::user());
        }

        return view('tournaments.show', compact('tournament', 'bracket', 'stats', 'userTeams', 'userSchedule', 'pending', 'approved'));
    }

    /**
     * Formulár na vytvorenie nového turnaja
     */
    public function create()
    {
        return view('tournaments.create');
    }

    /**
     * Ulož nový turnaj
     */
    public function store(Request $request)
    {
        $validated = $request->validate([
            'name' => 'required|string|max:200',
            'description' => 'nullable|string',
            'pricepool' => 'nullable|numeric|min:0',
            'type' => 'required|in:individual,team',
            'max_participants' => 'required|integer|min:2',
            'date' => 'required|date|after:today',
        ]);

        $tournament = Tournament::create([
            'name' => $validated['name'],
            'description' => $validated['description'],
            'pricepool' => $validated['pricepool'],
            'type' => $validated['type'],
            'max_participants' => $validated['max_participants'],
            'date' => $validated['date'],
            'manager_id' => Auth::id(),
            'approved' => false,
        ]);

        return redirect()
            ->route('tournaments.show', $tournament)
            ->with('success', 'Tournament has been created. It has to be approved by an administrator.');
    }

    /**
     * Formulár na úpravu turnaja
     */
    public function edit(Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        return view('tournaments.edit', compact('tournament'));
    }

    /**
     * Aktualizuj turnaj
     */
    public function update(Request $request, Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $validated = $request->validate([
            'name' => 'required|string|max:200',
            'description' => 'nullable|string',
            'pricepool' => 'required|numeric|min:0',
            'max_participants' => 'required|integer|min:2',
            'date' => 'required|date',
        ]);

        if ($tournament->participants()->exists()) {
            unset($validated['type']);
        }

        $tournament->update($validated);

        return redirect()
            ->to(route('tournaments.show', $tournament) . '#bracket')
            ->with('success', 'Tournament has been updated.');
    }

    /**
     * Vymaž turnaj
     */
    public function destroy(Tournament $tournament)
    {
        $this->authorize('delete', $tournament);

        $tournament->delete();

        return redirect()
            ->route('tournaments.index')
            ->with('success', 'Tournament has been deleted.');
    }

    /**
     * Schváľ turnaj
     */
    public function approve(Tournament $tournament)
    {
        $this->authorize('approve', $tournament);

        $tournament->update(['approved' => true]);

        return back()->with('success', 'Tournament has been approved.');
    }

    /**
     * Zamietni turnaj
     */
    public function reject(Tournament $tournament)
    {
        $this->authorize('approve', $tournament);

        $tournament->update(['approved' => false]);

        return back()->with('success', 'Tournament has been rejected.');
    }

    /**
     * Zoznam účastníkov turnaja
     */
    public function participants(Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $pending = $tournament->participants()
            ->with('participant')
            ->where('approved', false)
            ->get();

        $approved = $tournament->participants()
            ->with('participant')
            ->where('approved', true)
            ->get();

        return view('tournaments.participants', compact('tournament', 'pending', 'approved'));
    }

    /**
     * Schváľ účastníka turnaja
     */
    public function approveParticipant(Tournament $tournament, Participant $participant)
    {
        $this->authorize('update', $tournament);

        if ($participant->tournament_id !== $tournament->id) {
            abort(403);
        }

        $approvedCount = $tournament->participants()->where('approved', true)->count();
        if ($approvedCount >= $tournament->max_participants) {
            return back()->with('error', 'Turnaj je plný.');
        }

        $participant->update(['approved' => true]);

        return back()->with('success', 'Participant has been approved.');
    }

    /**
     * Zamietni účastníka turnaja
     */
    public function rejectParticipant(Tournament $tournament, Participant $participant)
    {
        $this->authorize('update', $tournament);

        if ($participant->tournament_id !== $tournament->id) {
            abort(403);
        }

        $participant->delete();

        return back()->with('success', 'Participant has been rejected.');
    }

    /**
     * Spusti turnaj a vygeneruje pavúka
     */
    public function start(Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        if (!$tournament->approved) {
            return back()->with('error', 'Tournament must be approved before starting.');
        }

        $approvedParticipants = $tournament->participants()
            ->where('approved', true)
            ->get();

        if ($approvedParticipants->count() < 2) {
            return back()->with('error', 'Tournament must have at least two approved participants to start.');
        }

        $this->createMatches($tournament, $approvedParticipants);

        return back()->with('success', 'Tournament has been started and match schedule generated.');
    }

    /**
     * Automaticky posuň hráčov, ktorí sú sami v zápase (BYE),
     * až kým je to možné (napr. 9 hráčov v 16-kovom pavúku).
     */

    /**
     * Automatické vylosovanie účastníkov
     */
    public function generateDraw(Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $participants = $tournament->participants()
            ->where('approved', true)
            ->get()
            ->shuffle();

        foreach ($participants as $index => $participant) {
            $participant->update(['draw_position' => $index + 1]);
        }

        return back()->with('success', 'Draw has been generated.');
    }

    /**
     * Štatistiky turnaja
     */
    public function statistics(Tournament $tournament)
    {
        $stats = [
            'total_participants' => $tournament->participants()->where('approved', true)->count(),
            'total_matches' => $tournament->matches()->count(),
            'completed_matches' => $tournament->matches()->where('finished', true)->count(),
            'pending_matches' => $tournament->matches()->where('finished', false)->count(),
            'top_scorers' => $this->getTopScorers($tournament),
            'average_score' => $tournament->matches()
                ->where('finished', true)
                ->avg(DB::raw('score1 + score2')),
        ];

        return view('tournaments.statistics', compact('tournament', 'stats'));
    }

    /**
     * Vytvor zápasy pre pavúka
     *
     * @param Tournament $tournament
     * @param Collection $participants
     */
    protected function createMatches(Tournament $tournament, $participants)
    {
        $participantCount = $participants->count();

        $bracketSize = pow(2, ceil(log($participantCount, 2)));

        $matchIndex = 0;
        for ($i = 0; $i < $bracketSize / 2; $i++) {
            $participant1 = $participants[$i * 2] ?? null;
            $participant2 = $participants[$i * 2 + 1] ?? null;

            TournamentMatch::create([
                'tournament_id' => $tournament->id,
                'participant1_id' => $participant1?->id,
                'participant2_id' => $participant2?->id,
                'index' => $matchIndex++,
                'finished' => false,
            ]);
        }

        $totalRounds = log($bracketSize, 2);
        for ($round = 2; $round <= $totalRounds; $round++) {
            $matchesInRound = $bracketSize / pow(2, $round);

            for ($m = 0; $m < $matchesInRound; $m++) {
                TournamentMatch::create([
                    'tournament_id' => $tournament->id,
                    'participant1_id' => null,
                    'participant2_id' => null,
                    'index' => $matchIndex++,
                    'finished' => false,
                ]);
            }
        }
    }

    /**
     * Vygeneruj štruktúru pavúka pre zobrazenie
     *
     * @param Tournament $tournament
     * @return array
     */
    protected function generateBracket(Tournament $tournament)
    {
        $matches = $tournament->matches()
            ->with(['participant1.participant', 'participant2.participant'])
            ->orderBy('index')
            ->get();

        if ($matches->isEmpty()) {
            return [];
        }

        $totalMatches = $matches->count();
        //$totalRounds = ceil(log($totalMatches + 1, 2));
        $firstRoundMatches = ($totalMatches + 1) / 2;
        $totalRounds = (int) log($totalMatches + 1, 2);

        $rounds = [];
        //$matchesPerRound = $totalMatches / 2;
        $startIndex = 0;

        for ($round = 1; $round <= $totalRounds; $round++) {
            //$roundMatches = $matches->slice($startIndex, $matchesPerRound)->values();
            //$rounds[$round] = $roundMatches;
            $matchesInRound = (int) ($firstRoundMatches / pow(2, $round - 1));
            $rounds[$round] = $matches->slice($startIndex, $matchesInRound)->values();
            $startIndex += $matchesInRound;

            //$startIndex += $matchesPerRound;
            //$matchesPerRound = $matchesPerRound / 2;
        }

        return $rounds;
    }
    /*
    protected function generateBracket(Tournament $tournament)
    {
        $matches = $tournament->matches()
            ->with(['participant1.participant', 'participant2.participant'])
            ->orderBy('index')
            ->get();

        if ($matches->isEmpty()) {
            return [];
        }

        $rounds = [];
        $roundNumber = 1;

        // Start with first round: matches with participants
        $firstRoundMatches = $matches->whereNotNull('participant1_id')->whereNotNull('participant2_id');
        $rounds[$roundNumber] = $firstRoundMatches->values();

        // Then assign remaining matches to later rounds
        $remainingMatches = $matches->diff($firstRoundMatches);

        while ($remainingMatches->isNotEmpty()) {
            $roundNumber++;
            $roundMatches = $remainingMatches->take($rounds[$roundNumber - 1]->count() / 2)->values();
            $rounds[$roundNumber] = $roundMatches;
            $remainingMatches = $remainingMatches->diff($roundMatches);
        }

        return $rounds;
    }
    */

    /**
     * Získaj harmonogram používateľa
     *
     * @param Tournament $tournament
     * @param User $user
     * @return Collection
     */
    protected function getUserSchedule(Tournament $tournament, User $user)
    {
        $userParticipants = $tournament->participants()
            ->where(function($query) use ($user) {
                $query->where('participant_type', User::class)
                      ->where('participant_id', $user->id);
            })
            ->orWhere(function($query) use ($user) {
                $query->where('participant_type', Team::class)
                      ->whereIn('participant_id', $user->teams()->pluck('teams.id'));
            })
            ->pluck('id');

        if ($userParticipants->isEmpty()) {
            return collect();
        }

        return $tournament->matches()
            ->where(function($query) use ($userParticipants) {
                $query->whereIn('participant1_id', $userParticipants)
                      ->orWhereIn('participant2_id', $userParticipants);
            })
            ->with(['participant1.participant', 'participant2.participant'])
            ->orderBy('index')
            ->get();
    }

    /**
     * Získaj top skórerov v turnaji
     *
     * @param Tournament $tournament
     * @return Collection
     */
    protected function getTopScorers(Tournament $tournament)
    {
        $matches = $tournament->matches()->where('finished', true)->get();

        $scores = [];

        foreach ($matches as $match) {
            if ($match->participant1_id) {
                $scores[$match->participant1_id] = ($scores[$match->participant1_id] ?? 0) + ($match->score1 ?? 0);
            }
            if ($match->participant2_id) {
                $scores[$match->participant2_id] = ($scores[$match->participant2_id] ?? 0) + ($match->score2 ?? 0);
            }
        }

        arsort($scores);

        return collect($scores)->take(10)->map(function($score, $participantId) {
            $participant = Participant::with('participant')->find($participantId);
            return [
                'participant' => $participant,
                'total_score' => $score
            ];
        });
    }
}
