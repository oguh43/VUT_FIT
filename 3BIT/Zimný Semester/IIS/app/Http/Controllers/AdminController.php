<?php

// AdminController.php
// Hugo Bohácsek (xbohach00)

namespace App\Http\Controllers;

use App\Models\User;
use App\Models\Tournament;
use App\Models\Team;
use App\Models\TournamentMatch;
use App\Models\Participant;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Illuminate\Support\Facades\DB;

class AdminController extends Controller
{
    /**
     * Administrátorský dashboard
     */
    public function dashboard()
    {
        $stats = [
            'total_users' => User::count(),
            'new_users_this_month' => User::whereMonth('created_at', now()->month)->count(),
            'total_tournaments' => Tournament::count(),
            'pending_tournaments' => Tournament::where('approved', false)->count(),
            'active_tournaments' => Tournament::where('approved', true)
                ->where('date', '>=', now())
                ->count(),
            'total_teams' => Team::count(),
            'total_matches' => TournamentMatch::count(),
            'completed_matches' => TournamentMatch::where('finished', true)->count(),
            'total_participants' => Participant::count(),
            'pending_participants' => Participant::where('approved', false)->count(),
        ];

        $recentUsers = User::orderBy('created_at', 'desc')->limit(10)->get();

        $pendingTournaments = Tournament::where('approved', false)
            ->with('creator')
            ->orderBy('created_at', 'desc')
            ->get();

        return view('admin.dashboard', compact('stats', 'recentUsers', 'pendingTournaments'));
    }

    /**
     * Správa používateľov
     */
    public function users(Request $request)
    {
        $query = User::query();

        // Filtrovanie
        if ($request->has('role')) {
            $query->where('role', $request->role);
        }

        if ($request->has('search')) {
            $query->where(function($q) use ($request) {
                $q->where('name', 'like', '%' . $request->search . '%')
                  ->orWhere('email', 'like', '%' . $request->search . '%');
            });
        }

        $users = $query->orderBy('created_at', 'desc')->paginate(20);

        return view('admin.users', compact('users'));
    }

    /**
     * Správa turnajov
     */
    public function tournaments(Request $request)
    {
        $query = Tournament::with('creator');

        // Filtrovanie
        if ($request->has('approved')) {
            $query->where('approved', $request->approved);
        }

        if ($request->has('type')) {
            $query->where('type', $request->type);
        }

        if ($request->has('search')) {
            $query->where('name', 'like', '%' . $request->search . '%');
        }

        $tournaments = $query->orderBy('created_at', 'desc')->paginate(20);

        return view('admin.tournaments', compact('tournaments'));
    }

    /**
     * Správa tímov
     */
    public function teams(Request $request)
    {
        $query = Team::withCount('members');

        if ($request->has('search')) {
            $query->where('name', 'like', '%' . $request->search . '%');
        }

        $teams = $query->orderBy('created_at', 'desc')->paginate(20);

        return view('admin.teams', compact('teams'));
    }

    /**
     * Zmena role používateľa
     */
    public function changeUserRole(Request $request, User $user)
    {
        if ($user->id === Auth::id()) {
            return back()->with('error', 'You cannot change your own role.');
        }

        $validated = $request->validate([
            'role' => 'required|in:user,admin',
        ]);

        $user->update(['role' => $validated['role']]);

        return back()->with('success', 'User role has been changed.');
    }

    /**
     * Hromadné schválenie turnajov
     */
    public function bulkApproveTournaments(Request $request)
    {
        $validated = $request->validate([
            'tournament_ids' => 'required|array',
            'tournament_ids.*' => 'exists:tournaments,id',
        ]);

        Tournament::whereIn('id', $validated['tournament_ids'])
            ->update(['approved' => true]);

        return back()->with('success', 'Selected tournaments have been approved.');
    }

    /**
     * Hromadné zamietnutie turnajov
     */
    public function bulkRejectTournaments(Request $request)
    {
        $validated = $request->validate([
            'tournament_ids' => 'required|array',
            'tournament_ids.*' => 'exists:tournaments,id',
        ]);

        Tournament::whereIn('id', $validated['tournament_ids'])
            ->update(['approved' => false]);

        return back()->with('success', 'Selected tournaments have been rejected.');
    }

    /**
     * Systémové štatistiky
     */
    public function statistics()
    {
        $stats = [
            // Používateľské štatistiky
            'user_growth' => $this->getUserGrowthByMonth(12),
            'users_by_role' => User::select('role', DB::raw('count(*) as count'))
                ->groupBy('role')
                ->pluck('count', 'role')
                ->toArray(),

            // Turnajové štatistiky
            'tournaments_by_month' => $this->getTournamentsByMonth(12),
            'tournaments_by_type' => Tournament::select('type', DB::raw('count(*) as count'))
                ->groupBy('type')
                ->pluck('count', 'type')
                ->toArray(),
            'tournaments_by_approval' => [
                'approved' => Tournament::where('approved', true)->count(),
                'pending' => Tournament::where('approved', false)->count(),
            ],

            // Týmové štatistiky
            'teams_by_size' => $this->getTeamsBySize(),
            'average_team_size' => Team::withCount('members')->avg('members_count'),

            // Zápasové štatistiky
            'matches_by_month' => $this->getMatchesByMonth(12),
            'matches_completion_rate' => $this->getMatchesCompletionRate(),

            // Účastnícke štatistiky
            'participants_by_type' => Participant::select('participant_type', DB::raw('count(*) as count'))
                ->groupBy('participant_type')
                ->get()
                ->mapWithKeys(function($item) {
                    $type = class_basename($item->participant_type);
                    return [$type => $item->count];
                })
                ->toArray(),
            'approval_rate' => $this->getApprovalRate(),
        ];

        return view('admin.statistics', compact('stats'));
    }

    /**
     * Export dát
     */
    public function export(Request $request)
    {
        $type = $request->get('type', 'users');

        switch ($type) {
            case 'users':
                return $this->exportUsers();
            case 'tournaments':
                return $this->exportTournaments();
            case 'teams':
                return $this->exportTeams();
            default:
                return back()->with('error', 'Unsupported export type.');
        }
    }

    /**
     * Vyčistenie starých dát
     */
    public function cleanup(Request $request)
    {
        $type = $request->get('type');

        switch ($type) {
            case 'old_tournaments':
                $deleted = Tournament::where('date', '<', now()->subYear())->delete();
                return back()->with('success', "Deleted {$deleted} old tournaments.");

            case 'rejected_participants':
                $deleted = Participant::where('approved', false)
                    ->where('created_at', '<', now()->subMonths(3))
                    ->delete();
                return back()->with('success', "Deleted {$deleted} rejected registration.");

            default:
                return back()->with('error', 'Unsupported cleanup type.');
        }
    }

    /**
     * Systémové informácie
     */
    public function systemInfo()
    {
        $info = [
            'php_version' => phpversion(),
            'laravel_version' => app()->version(),
            'database' => config('database.default'),
            'environment' => app()->environment(),
            'debug_mode' => config('app.debug') ? 'Enabled' : 'Disabled',
        ];

        return view('admin.system-info', compact('info'));
    }

    protected function getUserGrowthByMonth(int $months)
    {
        $data = [];
        for ($i = $months - 1; $i >= 0; $i--) {
            $date = now()->subMonths($i);
            $count = User::whereYear('created_at', $date->year)
                ->whereMonth('created_at', $date->month)
                ->count();
            $data[$date->format('M Y')] = $count;
        }
        return $data;
    }

    protected function getTournamentsByMonth(int $months)
    {
        $data = [];
        for ($i = $months - 1; $i >= 0; $i--) {
            $date = now()->subMonths($i);
            $count = Tournament::whereYear('created_at', $date->year)
                ->whereMonth('created_at', $date->month)
                ->count();
            $data[$date->format('M Y')] = $count;
        }
        return $data;
    }

    protected function getMatchesByMonth(int $months)
    {
        $data = [];
        for ($i = $months - 1; $i >= 0; $i--) {
            $date = now()->subMonths($i);
            $count = TournamentMatch::whereYear('created_at', $date->year)
                ->whereMonth('created_at', $date->month)
                ->count();
            $data[$date->format('M Y')] = $count;
        }
        return $data;
    }

    protected function getTeamsBySize()
    {
        return Team::withCount('members')
            ->get()
            ->groupBy(function($team) {
                $count = $team->members_count;
                if ($count <= 2) return '1-2';
                if ($count <= 5) return '3-5';
                if ($count <= 10) return '6-10';
                return '10+';
            })
            ->map->count()
            ->toArray();
    }

    protected function getMatchesCompletionRate()
    {
        $total = TournamentMatch::count();
        if ($total === 0) return 0;

        $completed = TournamentMatch::where('finished', true)->count();
        return round(($completed / $total) * 100, 2);
    }

    protected function getApprovalRate()
    {
        $total = Participant::count();
        if ($total === 0) return 0;

        $approved = Participant::where('approved', true)->count();
        return round(($approved / $total) * 100, 2);
    }

    protected function exportUsers()
    {
        $users = User::all();
        $filename = 'pouzivatelia_' . date('Y-m-d') . '.csv';

        $headers = [
            'Content-Type' => 'text/csv; charset=utf-8',
            'Content-Disposition' => 'attachment; filename="' . $filename . '"',
        ];

        $callback = function() use ($users) {
            $file = fopen('php://output', 'w');
            fprintf($file, chr(0xEF).chr(0xBB).chr(0xBF)); // BOM

            fputcsv($file, ['ID', 'Meno', 'Email', 'Rola', 'Vytvorené'], ';');

            foreach ($users as $user) {
                fputcsv($file, [
                    $user->id,
                    $user->name,
                    $user->email,
                    $user->role,
                    $user->created_at->format('d.m.Y H:i'),
                ], ';');
            }

            fclose($file);
        };

        return response()->stream($callback, 200, $headers);
    }

    protected function exportTournaments()
    {
        $tournaments = Tournament::with('creator')->get();
        $filename = 'turnaje_' . date('Y-m-d') . '.csv';

        $headers = [
            'Content-Type' => 'text/csv; charset=utf-8',
            'Content-Disposition' => 'attachment; filename="' . $filename . '"',
        ];

        $callback = function() use ($tournaments) {
            $file = fopen('php://output', 'w');
            fprintf($file, chr(0xEF).chr(0xBB).chr(0xBF)); // BOM

            fputcsv($file, ['ID', 'Názov', 'Typ', 'Schválený', 'Dátum', 'Správca'], ';');

            foreach ($tournaments as $tournament) {
                fputcsv($file, [
                    $tournament->id,
                    $tournament->name,
                    $tournament->type,
                    $tournament->approved ? 'Áno' : 'Nie',
                    $tournament->date->format('d.m.Y'),
                    $tournament->creator->name,
                ], ';');
            }

            fclose($file);
        };

        return response()->stream($callback, 200, $headers);
    }

    protected function exportTeams()
    {
        $teams = Team::withCount('members')->get();
        $filename = 'timy_' . date('Y-m-d') . '.csv';

        $headers = [
            'Content-Type' => 'text/csv; charset=utf-8',
            'Content-Disposition' => 'attachment; filename="' . $filename . '"',
        ];

        $callback = function() use ($teams) {
            $file = fopen('php://output', 'w');
            fprintf($file, chr(0xEF).chr(0xBB).chr(0xBF)); // BOM

            fputcsv($file, ['ID', 'Názov', 'Počet členov', 'Vytvorené'], ';');

            foreach ($teams as $team) {
                fputcsv($file, [
                    $team->id,
                    $team->name,
                    $team->members_count,
                    $team->created_at->format('d.m.Y H:i'),
                ], ';');
            }

            fclose($file);
        };

        return response()->stream($callback, 200, $headers);
    }
}
