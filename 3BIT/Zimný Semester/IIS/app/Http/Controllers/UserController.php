<?php

// UserController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\User;
use App\Models\Tournament;
use App\Models\Team;
use App\Models\Participant;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Facades\Storage;
use Illuminate\Validation\Rules\Password;
use Illuminate\Foundation\Auth\Access\AuthorizesRequests;

class UserController extends Controller
{
    use AuthorizesRequests;
    /**
     * Zobraz zoznam všetkých používateľov
     */
    public function index(Request $request)
    {
        $query = User::query();

        if ($request->has('search')) {
            $query->where('name', 'like', '%' . $request->search . '%')
                  ->orWhere('email', 'like', '%' . $request->search . '%');
        }

        if ($request->has('role')) {
            $query->where('role', $request->role);
        }

        $users = $query->orderBy('name')->paginate(20);

        return view('users.index', compact('users'));
    }

    /**
     * Zobraz profil používateľa vrátane štatistík
     */
    public function show(User $user)
    {
        $user->load(['createdTeams', 'teams', 'createdTournaments']);

        $stats = $this->getUserStatistics($user);

        $recentMatches = $this->getUserRecentMatches($user, 10);
        $upcomingMatches = $this->getUserUpcomingMatches($user);
        $pastMatches = $this->getUserRecentMatches($user, 20);

        return view('users.show', compact('user', 'stats', 'recentMatches', 'upcomingMatches', 'pastMatches'));
    }

    /**
     * Formulár na úpravu profilu
     */
    public function edit(User $user)
    {
        $this->authorize('update', $user);

        return view('users.edit', compact('user'));
    }

    /**
     * Aktualizuj profil používateľa
     */
    public function update(Request $request, User $user)
    {
        $this->authorize('update', $user);

        $validated = $request->validate([
            'name' => 'required|string|max:255',
            'email' => 'required|email|unique:users,email,' . $user->id,
            'image' => 'nullable|image|mimes:jpeg,png,jpg,gif|max:2048',
        ]);

        if ($request->hasFile('image')) {
            if ($user->image != "img/default_user.png") {
                Storage::disk('public')->delete(str_replace('storage/', '', $user->image));
            }

            $validated['image'] = 'storage/' . $request->file('image')->store('profile-pictures', 'public');
        }

        $user->update($validated);

        return redirect()
            ->route('users.show', $user)
            ->with('success', 'Profile has been updated.');
    }

    /**
     * Formulár na zmenu hesla
     */
    public function editPassword(User $user)
    {
        $this->authorize('update', $user);

        return view('users.edit-password', compact('user'));
    }

    /**
     * Zmení heslo používateľa
     */
    public function updatePassword(Request $request, User $user)
    {
        $this->authorize('update', $user);

        $validated = $request->validate([
            'current_password' => 'required',
            'password' => ['required', 'confirmed', Password::min(8)],
        ]);

        if (!Hash::check($validated['current_password'], $user->password)) {
            return back()->with('error', 'Current password is incorrect.');
        }

        $user->update([
            'password' => Hash::make($validated['password'])
        ]);

        return redirect()
            ->route('users.show', $user)
            ->with('success', 'Password has been updated.');
    }

    /**
     * Vymaž používateľa
     */
    public function destroy(User $user)
    {
        $this->authorize('delete', $user);

        if ($user->id === Auth::id()) {
            return back()->with('error', 'You cannot delete your own account.');
        }

        if ($user->image) {
            Storage::disk('public')->delete(str_replace('storage/', '', $user->image));
        }

        $user->delete();

        return redirect()
            ->route('users.index')
            ->with('success', 'User has been deleted.');
    }

    /**
     * Štatistiky používateľa
     */
    public function statistics(User $user)
    {
        $stats = $this->getUserStatistics($user);

        $extendedStats = [
            'tournaments_by_type' => $this->getTournamentsByType($user),
            'monthly_activity' => $this->getMonthlyActivity($user),
            'team_contributions' => $this->getTeamContributions($user),
        ];

        return view('users.statistics', compact('user', 'stats', 'extendedStats'));
    }

    /**
     * Harmonogram používateľa
     */
    public function schedule(User $user)
    {
        $this->authorize('view-schedule', $user);

        $upcomingMatches = $this->getUserUpcomingMatches($user);

        $pastMatches = $this->getUserRecentMatches($user, 20);

        return view('users.schedule', compact('user', 'upcomingMatches', 'pastMatches'));
    }

    /**
     * Získaj štatistiky používateľa
     *
     * @param User $user
     * @return array
     */
    protected function getUserStatistics(User $user)
    {
        $userParticipations = Participant::where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->where('approved', true)
            ->pluck('id');

        $teamParticipations = Participant::where('participant_type', Team::class)
            ->whereIn('participant_id', $user->teams()->pluck('teams.id'))
            ->where('approved', true)
            ->pluck('id');

        $allParticipations = $userParticipations->merge($teamParticipations);

        $totalMatches = \App\Models\TournamentMatch::where(function($query) use ($allParticipations) {
                $query->whereIn('participant1_id', $allParticipations)
                      ->orWhereIn('participant2_id', $allParticipations);
            })
            ->where('finished', true)
            ->count();

        $matchesWon = \App\Models\TournamentMatch::whereIn('winner_id', $allParticipations)
            ->where('finished', true)
            ->count();

        $winRate = $totalMatches > 0 ? round(($matchesWon / $totalMatches) * 100, 2) : 0;

        $totalTournaments = Participant::whereIn('id', $allParticipations)->count();

        return [
            'total_tournaments' => $totalTournaments,
            'total_matches' => $totalMatches,
            'matches_won' => $matchesWon,
            'matches_lost' => $totalMatches - $matchesWon,
            'win_rate' => $winRate,
            'teams_count' => $user->teams()->count(),
            'created_teams' => $user->createdTeams()->count(),
            'created_tournaments' => $user->createdTournaments()->count(),
        ];
    }

    /**
     * Získaj nedávne zápasy používateľa
     *
     * @param User $user
     * @param int $limit
     * @return Collection
     */
    protected function getUserRecentMatches(User $user, int $limit = 10)
    {
        $userParticipations = Participant::where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->where('approved', true)
            ->pluck('id');

        $teamParticipations = Participant::where('participant_type', Team::class)
            ->whereIn('participant_id', $user->teams()->pluck('teams.id'))
            ->where('approved', true)
            ->pluck('id');

        $allParticipations = $userParticipations->merge($teamParticipations);

        if ($allParticipations->isEmpty()) {
            return collect();
        }

        return \App\Models\TournamentMatch::where(function($query) use ($allParticipations) {
                $query->whereIn('participant1_id', $allParticipations)
                      ->orWhereIn('participant2_id', $allParticipations);
            })
            ->where('finished', true)
            ->with(['tournament', 'participant1.participant', 'participant2.participant'])
            ->orderBy('updated_at', 'desc')
            ->limit($limit)
            ->get();
    }

    /**
     * Získaj nadchádzajúce zápasy používateľa
     *
     * @param User $user
     * @return Collection
     */
    protected function getUserUpcomingMatches(User $user)
    {
        $userParticipations = Participant::where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->where('approved', true)
            ->pluck('id');

        $teamParticipations = Participant::where('participant_type', Team::class)
            ->whereIn('participant_id', $user->teams()->pluck('teams.id'))
            ->where('approved', true)
            ->pluck('id');

        $allParticipations = $userParticipations->merge($teamParticipations);

        if ($allParticipations->isEmpty()) {
            return collect();
        }

        return \App\Models\TournamentMatch::where(function($query) use ($allParticipations) {
                $query->whereIn('participant1_id', $allParticipations)
                      ->orWhereIn('participant2_id', $allParticipations);
            })
            ->where('finished', false)
            ->whereHas('tournament', function($query) {
                $query->where('date', '>=', now());
            })
            ->with(['tournament', 'participant1.participant', 'participant2.participant'])
            ->orderBy('created_at')
            ->get();
    }

    /**
     * Získaj turnaje podľa typu
     *
     * @param User $user
     * @return array
     */
    protected function getTournamentsByType(User $user)
    {
        $userParticipations = Participant::where('participant_type', User::class)
            ->where('participant_id', $user->id)
            ->where('approved', true)
            ->with('tournament')
            ->get();

        $individual = $userParticipations->filter(function($p) {
            return $p->tournament->type === 'individual';
        })->count();

        $team = $userParticipations->filter(function($p) {
            return $p->tournament->type === 'team';
        })->count();

        return [
            'individual' => $individual,
            'team' => $team,
        ];
    }

    /**
     * Získaj mesačnú aktivitu - TODO
     *
     * @param User $user
     * @return array
     */
    protected function getMonthlyActivity(User $user)
    {
        // TODO: Implementovať
        return [];
    }

    /**
     * Získa príspevky v tímoch TODO
     *
     * @param User $user
     * @return Collection
     */
    protected function getTeamContributions(User $user)
    {
        return $user->teams->map(function($team) use ($user) {
            // TODO: Implementovať
            return [
                'team' => $team,
                'matches_played' => 0,
                'contribution' => 0,
            ];
        });
    }
}
