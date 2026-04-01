<?php

// ParticipantController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\Tournament;
use App\Models\Participant;
use App\Models\User;
use App\Models\Team;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Foundation\Auth\Access\AuthorizesRequests;

class ParticipantController extends Controller
{
    use AuthorizesRequests;
    /**
     * Formulár na registráciu na turnaj
     */
    public function create(Tournament $tournament)
    {
        if (!$tournament->approved) {
            return redirect()
                ->route('tournaments.show', $tournament)
                ->with('error', 'Tournament is not approved yet.');
        }

        if ($this->isTournamentFull($tournament)) {
            return redirect()
                ->route('tournaments.show', $tournament)
                ->with('error', 'Tournament is full.');
        }

        $userTeams = null;
        if ($tournament->type === 'team') {
            $userTeams = Auth::user()->createdTeams;
        }

        return view('participants.create', compact('tournament', 'userTeams'));
    }

    /**
     * Registrácia jednotlivca na turnaj
     */
    public function storeIndividual(Tournament $tournament)
    {
        if ($tournament->type !== 'individual') {
            return back()->with('error', 'This tournament is only for teams.');
        }

        if (!$tournament->approved) {
            return back()->with('error', 'Tournament is not approved yet.');
        }

        if ($this->isTournamentFull($tournament)) {
            return back()->with('error', 'Tournament is full.');
        }

        $user = Auth::user();

        $existingParticipation = $tournament->participants()
            ->where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->exists();

        if ($existingParticipation) {
            return back()->with('error', 'You are already registered for this tournament.');
        }

        Participant::create([
            'tournament_id' => $tournament->id,
            'participant_type' => User::class,
            'participant_id' => $user->id,
            'approved' => false,
        ]);

        return redirect()
            ->route('tournaments.show', $tournament)
            ->with('success', 'Your registration has been submitted for approval.');
    }

    /**
     * Registrácia tímu na turnaj
     */
    public function storeTeam(Request $request, Tournament $tournament)
    {
        if ($tournament->type !== 'team') {
            return back()->with('error', 'This tournament is only for individuals.');
        }

        if (!$tournament->approved) {
            return back()->with('error', 'Tournament is not approved yet.');
        }

        if ($this->isTournamentFull($tournament)) {
            return back()->with('error', 'Tournament is full.');
        }

        $validated = $request->validate([
            'team_id' => 'required|exists:teams,id',
        ]);

        $team = Team::findOrFail($validated['team_id']);

        if ($team->manager_id !== Auth::id()) {
            return back()->with('error', 'Only the team manager can register for this tournament.');
        }

        $existingParticipation = $tournament->participants()
            ->where('participant_type', Team::class)
            ->where('participant_id', $team->id)
            ->exists();

        if ($existingParticipation) {
            return back()->with('error', 'This team is already registered for this tournament.');
        }

        if ($team->members()->count() === 0) {
            return back()->with('error', 'Team must have at least 1 member.');
        }

        Participant::create([
            'tournament_id' => $tournament->id,
            'participant_type' => Team::class,
            'participant_id' => $team->id,
            'approved' => false,
        ]);

        return redirect()
            ->route('tournaments.show', $tournament)
            ->with('success', 'Team registration has been submitted for approval.');
    }

    /**
     * Zrušenie registrácie
     */
    public function destroy(Participant $participant)
    {
        if (!$this->canCancelRegistration($participant)) {
            abort(403, 'You are not authorized to cancel this registration.');
        }

        $tournament = $participant->tournament;

        if ($tournament->date < now()) {
            return back()->with('error', 'Registration cannot be canceled after the tournament has started.');
        }

        if ($tournament->matches()->exists()) {
            return back()->with('error', 'Registration cannot be canceled, matches have already been generated.');
        }

        $participant->delete();

        return redirect()
            ->route('tournaments.show', $tournament)
            ->with('success', 'Registration canceled.');
    }

    /**
     * Zobrazie registrácie používateľa
     */
    public function myParticipations()
    {
        $user = Auth::user();

        $individualParticipations = Participant::where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->with('tournament')
            ->get();

        $teamParticipations = Participant::where('participant_type', Team::class)
            ->whereIn('participant_id', $user->createdTeams()->pluck('id'))
            ->with(['tournament', 'participant'])
            ->get();

        return view('participants.my-participations', compact('individualParticipations', 'teamParticipations'));
    }

    /**
     * Hromadné schválenie registrácií
     */
    public function bulkApprove(Request $request, Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $validated = $request->validate([
            'participant_ids' => 'required|array',
            'participant_ids.*' => 'exists:participants,id',
        ]);

        $currentApproved = $tournament->participants()->where('approved', true)->count();
        $toApprove = count($validated['participant_ids']);

        if ($currentApproved + $toApprove > $tournament->max_participants) {
            return back()->with('error', 'The tournament capacity would have been exceeded.');
        }

        Participant::whereIn('id', $validated['participant_ids'])
            ->where('tournament_id', $tournament->id)
            ->update(['approved' => true]);

        return back()->with('success', 'Selected registrations have been approved.');
    }

    /**
     * Hromadné zamietnutie registrácií
     */
    public function bulkReject(Request $request, Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $validated = $request->validate([
            'participant_ids' => 'required|array',
            'participant_ids.*' => 'exists:participants,id',
        ]);

        // Zmaže vybrané registrácie
        Participant::whereIn('id', $validated['participant_ids'])
            ->where('tournament_id', $tournament->id)
            ->delete();

        return back()->with('success', 'Selected registrations have been rejected.');
    }

    /**
     * Export registrácií do CSV
     */
    public function export(Tournament $tournament)
    {
        $this->authorize('update', $tournament);

        $participants = $tournament->participants()->with('participant')->get();

        $filename = 'registracie_turnaj_' . $tournament->id . '_' . date('Y-m-d') . '.csv';

        $headers = [
            'Content-Type' => 'text/csv; charset=utf-8',
            'Content-Disposition' => 'attachment; filename="' . $filename . '"',
        ];

        $callback = function() use ($participants) {
            $file = fopen('php://output', 'w');

            // BOM pre renderovanie diakritiky (smol trik)
            fprintf($file, chr(0xEF).chr(0xBB).chr(0xBF));

            fputcsv($file, ['ID', 'Typ', 'Názov', 'Schválené', 'Dátum registrácie'], ';');

            foreach ($participants as $participant) {
                $type = $participant->participant_type === User::class ? 'Jednotlivec' : 'Tím';
                $name = $participant->participant->name;
                $approved = $participant->approved ? 'Áno' : 'Nie';

                fputcsv($file, [
                    $participant->id,
                    $type,
                    $name,
                    $approved,
                    $participant->created_at->format('d.m.Y H:i'),
                ], ';');
            }

            fclose($file);
        };

        return response()->stream($callback, 200, $headers);
    }

    /**
     * Je turnaj plný?
     *
     * @param Tournament $tournament
     * @return bool
     */
    protected function isTournamentFull(Tournament $tournament): bool
    {
        $approvedCount = $tournament->participants()->where('approved', true)->count();
        return $approvedCount >= $tournament->max_participants;
    }

    /**
     * Môže používateľ zrušiť registráciu?
     *
     * @param Participant $participant
     * @return bool
     */
    protected function canCancelRegistration(Participant $participant): bool
    {
        $user = Auth::user();

        if ($participant->participant_type === User::class) {
            return $participant->participant_id === $user->id;
        }

        if ($participant->participant_type === Team::class) {
            $team = Team::find($participant->participant_id);
            return $team && $team->manager_id === $user->id;
        }

        return false;
    }

    /**
     * Kontrola dostupnosti - cez AJAX
     */
    public function checkAvailability(Tournament $tournament)
    {
        $approvedCount = $tournament->participants()->where('approved', true)->count();
        $available = $approvedCount < $tournament->max_participants;
        $spotsLeft = $tournament->max_participants - $approvedCount;

        return response()->json([
            'available' => $available,
            'spots_left' => $spotsLeft,
            'max_participants' => $tournament->max_participants,
            'current_participants' => $approvedCount,
        ]);
    }
}
