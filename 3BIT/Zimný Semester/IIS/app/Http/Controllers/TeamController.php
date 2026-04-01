<?php

// TeamController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\Team;
use App\Models\User;
use App\Models\Participant;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\Storage;
use Illuminate\Foundation\Auth\Access\AuthorizesRequests;

class TeamController extends Controller
{
    use AuthorizesRequests;
    /**
     * Zoznam všetkých tímov
     */
    public function index(Request $request)
    {
        $query = Team::withCount('members');

        if ($request->has('search')) {
            $query->where('name', 'like', '%' . $request->search . '%');
        }

        $teams = $query->orderBy('name')->paginate(12);

        return view('teams.index', compact('teams'));
    }

    /**
     * Detail tímu vrátane členov a štatistík
     */
    public function show(Team $team)
    {
        $team->load(['manager', 'members', 'tournaments.tournament']);

        $stats = $this->getTeamStatistics($team);

        $recentMatches = $this->getTeamRecentMatches($team, 10);

        $availableUsers = $this->getAvailableUsers($team);

        return view('teams.show', compact('team', 'stats', 'recentMatches', 'availableUsers'));
    }

    /**
     * Formulár na vytvorenie nového tímu
     */
    public function create()
    {
        return view('teams.create');
    }

    /**
     * Ulož nový tím
     */
    public function store(Request $request)
    {
        $validated = $request->validate([
            'name' => 'required|string|max:100|unique:teams,name',
            'image' => 'nullable|image|mimes:jpeg,png,jpg,gif,svg|max:2048',
        ]);

        // Logo
        $imagePath = null;
        if ($request->hasFile('image')) {
            $imagePath = 'storage/' . $request->file('image')->store('team-logos', 'public');
        }

        if ($imagePath === null){
            $team = Team::create([
                'name' => $validated['name'],
                'manager_id' => Auth::id(),
            ]);
        }else {
            $team = Team::create([
                'name' => $validated['name'],
                'image' => $imagePath,
                'manager_id' => Auth::id(),
            ]);
        }

        $team->members()->attach(Auth::id());

        return redirect()
            ->route('teams.show', $team)
            ->with('success', 'Team has been created. You are the manager of the team.');
    }

    /**
     * Formulár na úpravu tímu
     */
    public function edit(Team $team)
    {
        $this->authorize('update', $team);

        return view('teams.edit', compact('team'));
    }

    /**
     * Aktualizuj tím
     */
    public function update(Request $request, Team $team)
    {
        $this->authorize('update', $team);

        $validated = $request->validate([
            'name' => 'required|string|max:100|unique:teams,name,' . $team->id,
            'image' => 'nullable|image|mimes:jpeg,png,jpg,gif,svg|max:2048',
        ]);

        // Nové logo?
        if ($request->hasFile('image')) {
            if ($team->image) {
                Storage::disk('public')->delete(str_replace('storage/', '', $team->image));
            }

            $validated['image'] = 'storage/' . $request->file('image')->store('team-logos', 'public');
        }

        $team->update($validated);

        return redirect()
            ->route('teams.show', $team)
            ->with('success', 'Team has been updated.');
    }

    /**
     * Vymaž tím
     */
    public function destroy(Team $team)
    {
        $this->authorize('delete', $team);

        $activeTournaments = $team->tournaments()
            ->whereHas('tournament', function($query) {
                $query->where('approved', true)
                      ->where('date', '>=', now());
            })
            ->count();

        if ($activeTournaments > 0) {
            return back()->with('error', 'Team cannot be deleted because it is registered in active tournaments.');
        }

        if ($team->image) {
            Storage::disk('public')->delete(str_replace('storage/', '', $team->image));
        }

        $team->delete();

        return redirect()
            ->route('teams.index')
            ->with('success', 'Team has been deleted.');
    }

    /**
     * Zobraz členov tímu
     */
    public function members(Team $team)
    {
        $members = $team->members()->withPivot('created_at')->get();

        $availableUsers = null;
        if (Auth::check() && Auth::id() === $team->manager_id) {
            $availableUsers = User::whereNotIn('id', $members->pluck('id'))
                ->orderBy('name')
                ->get();
        }

        return view('teams.members', compact('team', 'members', 'availableUsers'));
    }

    /**
     * Pridaj člena do tímu
     */
    public function addMember(Request $request, Team $team)
    {
        $this->authorize('update', $team);

        $validated = $request->validate([
            'user_id' => 'required|exists:users,id',
        ]);

        if ($team->members()->where('user_id', $validated['user_id'])->exists()) {
            return back()->with('error', 'User is already a member of this team.');
        }

        $team->members()->attach($validated['user_id']);

        return back()->with('success', 'User has been added to the team.');
    }

    /**
     * Zobraz uzivatele, co jeste nejsou cleny tymu
    */
    protected function getAvailableUsers(Team $team)
    {
        return User::whereNotIn('id', $team->members()->pluck('users.id'))->get();
    }

    /**
     * Odober člena z tímu
     */
    public function removeMember(Team $team, User $user)
    {
        $this->authorize('update', $team);

        if ($team->manager_id === $user->id) {
            return back()->with('error', 'Team manager cannot be removed.');
        }

        $team->members()->detach($user->id);

        return back()->with('success', 'Team member has been removed.');
    }

    /**
     * Používateľ opustí tím
     */
    public function leave(Team $team)
    {
        $user = Auth::user();

        if ($team->manager_id === $user->id) {
            return back()->with('error', 'Team manager cannot leave the team. You must transfer ownership first.');
        }

        if (!$team->members()->where('user_id', $user->id)->exists()) {
            return back()->with('error', 'You are not a member of this team.');
        }

        $team->members()->detach($user->id);

        return redirect()
            ->route('teams.index')
            ->with('success', 'You have left the team.');
    }

    /**
     * Presuň správu tímu na iného člena
     */
    public function transferOwnership(Request $request, Team $team)
    {
        $this->authorize('update', $team);

        $validated = $request->validate([
            'new_manager_id' => 'required|exists:users,id',
        ]);

        if (!$team->members()->where('user_id', $validated['new_manager_id'])->exists()) {
            return back()->with('error', 'New manager must be a member of the team.');
        }

        $team->update(['manager_id' => $validated['new_manager_id']]);

        return back()->with('success', 'Team ownership has been transferred to the new manager.');
    }

    /**
     * Štatistiky tímu
     */
    public function statistics(Team $team)
    {
        $stats = $this->getTeamStatistics($team);

        $memberStats = $this->getMemberStatistics($team);

        return view('teams.statistics', compact('team', 'stats', 'memberStats'));
    }

    /**
     * Získaj štatistiky tímu
     *
     * @param Team $team
     * @return array
     */
    protected function getTeamStatistics(Team $team)
    {
        $participations = $team->tournaments()->with('tournament')->get();

        $totalMatches = 0;
        $matchesWon = 0;
        $totalScore = 0;
        $totalConceded = 0;

        foreach ($participations as $participation) {
            $matches = $participation->tournament->matches()
                ->where(function($query) use ($participation) {
                    $query->where('participant1_id', $participation->id)
                          ->orWhere('participant2_id', $participation->id);
                })
                ->where('finished', true)
                ->get();

            foreach ($matches as $match) {
                $totalMatches++;

                if ($match->participant1_id === $participation->id) {
                    $totalScore += $match->score1 ?? 0;
                    $totalConceded += $match->score2 ?? 0;
                    if ($match->winner_id === $participation->id) {
                        $matchesWon++;
                    }
                } else {
                    $totalScore += $match->score2 ?? 0;
                    $totalConceded += $match->score1 ?? 0;
                    if ($match->winner_id === $participation->id) {
                        $matchesWon++;
                    }
                }
            }
        }

        $winRate = $totalMatches > 0 ? round(($matchesWon / $totalMatches) * 100, 2) : 0;
        $avgScore = $totalMatches > 0 ? round($totalScore / $totalMatches, 2) : 0;

        return [
            'total_tournaments' => $participations->count(),
            'active_tournaments' => $participations->filter(function($p) {
                return $p->tournament->date >= now() && $p->tournament->approved;
            })->count(),
            'total_matches' => $totalMatches,
            'matches_won' => $matchesWon,
            'matches_lost' => $totalMatches - $matchesWon,
            'win_rate' => $winRate,
            'total_score' => $totalScore,
            'total_conceded' => $totalConceded,
            'average_score' => $avgScore,
            'members_count' => $team->members()->count(),
        ];
    }

    /**
     * Získaj štatistiky členov tímu
     *
     * @param Team $team
     * @return Collection
     */
    protected function getMemberStatistics(Team $team)
    {
        return $team->members->map(function($member) use ($team) {
            return [
                'user' => $member,
                'matches_played' => 0, // TODO: implementovať
                'goals_scored' => 0,   // TODO: implementovať
            ];
        });
    }

    /**
     * Získaj nedávne zápasy tímu
     *
     * @param Team $team
     * @param int $limit
     * @return Collection
     */
    protected function getTeamRecentMatches(Team $team, int $limit = 10)
    {
        $participations = $team->tournaments()->pluck('id');

        if ($participations->isEmpty()) {
            return collect();
        }

        return \App\Models\TournamentMatch::whereHas('tournament', function($query) {
                $query->where('date', '<=', now());
            })
            ->where(function($query) use ($participations) {
                $query->whereIn('participant1_id', $participations)
                      ->orWhereIn('participant2_id', $participations);
            })
            ->where('finished', true)
            ->with(['tournament', 'participant1.participant', 'participant2.participant'])
            ->orderBy('updated_at', 'desc')
            ->limit($limit)
            ->get();
    }
}
