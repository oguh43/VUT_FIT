<?php

// routes/web.php
// Filip Jenis (xjenisf00)
// Hugo Bohácsek (xbohach00)

use Illuminate\Support\Facades\Route;

use App\Http\Controllers\AuthController;


use App\Http\Controllers\TournamentController;
use App\Http\Controllers\TeamController;
use App\Http\Controllers\UserController;
use App\Http\Controllers\MatchController;
use App\Http\Controllers\ParticipantController;
use App\Http\Controllers\AdminController;

Route::get('/', function () {
    return view('index');
});

Route::middleware(['guest'])->group(function () {
    Route::get('/register', [AuthController::class, 'register'])->name('register');
    Route::post('/register', [AuthController::class, 'store']);
    Route::get('/login', [AuthController::class, 'login'])->name('login');
    Route::post('/login', [AuthController::class, 'authenticate']);
});

Route::middleware(['auth'])->group(function () {
    Route::post('/logout', [AuthController::class, 'logout'])->name('logout');

    Route::get('/tournaments/create', [TournamentController::class, 'create'])->name('tournaments.create');
    Route::post('/tournaments', [TournamentController::class, 'store'])->name('tournaments.store');

    Route::get('/tournaments/{tournament}/edit', [TournamentController::class, 'edit'])->name('tournaments.edit');
    Route::put('/tournaments/{tournament}', [TournamentController::class, 'update'])->name('tournaments.update');
    Route::delete('/tournaments/{tournament}', [TournamentController::class, 'destroy'])->name('tournaments.destroy');

    Route::get('/tournaments/{tournament}/participants', [TournamentController::class, 'participants'])->name('tournaments.participants');
    Route::post('/tournaments/{tournament}/participants/{participant}/approve', [TournamentController::class, 'approveParticipant'])->name('tournaments.participants.approve');
    Route::post('/tournaments/{tournament}/participants/{participant}/reject', [TournamentController::class, 'rejectParticipant'])->name('tournaments.participants.reject');

    Route::post('/tournaments/{tournament}/start', [TournamentController::class, 'start'])->name('tournaments.start');
    Route::post('/tournaments/{tournament}/generate-draw', [TournamentController::class, 'generateDraw'])->name('tournaments.generate-draw');

    Route::get('/teams/create', [TeamController::class, 'create'])->name('teams.create');
    Route::post('/teams', [TeamController::class, 'store'])->name('teams.store');

    Route::get('/teams/{team}/edit', [TeamController::class, 'edit'])->name('teams.edit');
    Route::post('/teams/{team}', [TeamController::class, 'update'])->name('teams.update');
    Route::delete('/teams/{team}', [TeamController::class, 'destroy'])->name('teams.destroy');

    Route::post('/teams/{team}/members', [TeamController::class, 'addMember'])->name('teams.members.add');
    Route::delete('/teams/{team}/members/{user}', [TeamController::class, 'removeMember'])->name('teams.members.remove');
    Route::post('/teams/{team}/leave', [TeamController::class, 'leave'])->name('teams.leave');
    Route::post('/teams/{team}/transfer-ownership', [TeamController::class, 'transferOwnership'])->name('teams.transfer-ownership');

    Route::get('/users/{user}/edit', [UserController::class, 'edit'])->name('users.edit');
    Route::post('/users/{user}', [UserController::class, 'update'])->name('users.update');

    Route::get('/users/{user}/edit-password', [UserController::class, 'editPassword'])->name('users.edit-password');
    Route::post('/users/{user}/password', [UserController::class, 'updatePassword'])->name('users.update-password');

    Route::get('/users/{user}/schedule', [UserController::class, 'schedule'])->name('users.schedule');

    Route::get('/matches/{match}/edit', [MatchController::class, 'edit'])->name('matches.edit');
    Route::put('/matches/{match}', [MatchController::class, 'update'])->name('matches.update');
    Route::post('/matches/{match}/quick-update', [MatchController::class, 'quickUpdate'])->name('matches.quick-update');
    Route::post('/matches/{match}/reset', [MatchController::class, 'reset'])->name('matches.reset');
    Route::delete('/matches/{match}', [MatchController::class, 'destroy'])->name('matches.destroy');

    Route::get('/tournaments/{tournament}/register', [ParticipantController::class, 'create'])->name('participants.create');
    Route::post('/tournaments/{tournament}/register/individual', [ParticipantController::class, 'storeIndividual'])->name('participants.store.individual');
    Route::post('/tournaments/{tournament}/register/team', [ParticipantController::class, 'storeTeam'])->name('participants.store.team');

    Route::post('/tournaments/{tournament}/start', [TournamentController::class, 'start'])->name('tournaments.start');

    Route::post('/tournaments/{tournament}/participants/individual', [ParticipantController::class, 'storeIndividual'])->name('participants.storeIndividual');
    Route::post('/tournaments/{tournament}/participants/team', [ParticipantController::class, 'storeTeam'])->name('participants.storeTeam');
    Route::post('/tournaments/{tournament}/participants/{participant}/approve', [TournamentController::class, 'approveParticipant'])->name('participants.approve');
    Route::delete('/tournaments/{tournament}/participants/{participant}/reject', [TournamentController::class, 'rejectParticipant'])->name('participants.reject');
    Route::delete('/participants/{participant}', [ParticipantController::class, 'destroy'])->name('participants.destroy');

    Route::get('/my-participations', [ParticipantController::class, 'myParticipations'])->name('participants.my');

    Route::post('/tournaments/{tournament}/participants/bulk-approve', [ParticipantController::class, 'bulkApprove'])->name('participants.bulk-approve');
    Route::post('/tournaments/{tournament}/participants/bulk-reject', [ParticipantController::class, 'bulkReject'])->name('participants.bulk-reject');

    Route::get('/tournaments/{tournament}/participants/export', [ParticipantController::class, 'export'])->name('participants.export');

    Route::get('/tournaments/{tournament}/check-availability', [ParticipantController::class, 'checkAvailability'])->name('participants.check-availability');
});

Route::middleware(['auth', 'admin'])->prefix('admin')->name('admin.')->group(function () {
    Route::get('/', [AdminController::class, 'dashboard'])->name('dashboard');

    Route::get('/users', [AdminController::class, 'users'])->name('users');
    Route::post('/users/{user}/change-role', [AdminController::class, 'changeUserRole'])->name('users.change-role');
    Route::delete('/users/{user}', [UserController::class, 'destroy'])->name('users.destroy');

    Route::get('/tournaments', [AdminController::class, 'tournaments'])->name('tournaments');
    Route::post('/tournaments/{tournament}/approve', [TournamentController::class, 'approve'])->name('tournaments.approve');
    Route::post('/tournaments/{tournament}/reject', [TournamentController::class, 'reject'])->name('tournaments.reject');
    Route::post('/tournaments/bulk-approve', [AdminController::class, 'bulkApproveTournaments'])->name('tournaments.bulk-approve');
    Route::post('/tournaments/bulk-reject', [AdminController::class, 'bulkRejectTournaments'])->name('tournaments.bulk-reject');

    Route::get('/teams', [AdminController::class, 'teams'])->name('teams');

    Route::get('/statistics', [AdminController::class, 'statistics'])->name('statistics');

    Route::get('/export', [AdminController::class, 'export'])->name('export');

    Route::post('/cleanup', [AdminController::class, 'cleanup'])->name('cleanup');

    Route::get('/system-info', [AdminController::class, 'systemInfo'])->name('system-info');
});

Route::get('/tournaments', [TournamentController::class, 'index'])->name('tournaments.index');
Route::get('/tournaments/{tournament}', [TournamentController::class, 'show'])->name('tournaments.show');
Route::get('/tournaments/{tournament}#bracket', [TournamentController::class, 'show'])->name('tournaments.show#bracket');
Route::get('/tournaments/{tournament}/statistics', [TournamentController::class, 'statistics'])->name('tournaments.statistics');

Route::get('/teams', [TeamController::class, 'index'])->name('teams.index');
Route::get('/teams/{team}', [TeamController::class, 'show'])->name('teams.show');
Route::get('/teams/{team}/members', [TeamController::class, 'members'])->name('teams.members');
Route::get('/teams/{team}/statistics', [TeamController::class, 'statistics'])->name('teams.statistics');

Route::get('/users', [UserController::class, 'index'])->name('users.index');
Route::get('/users/{user}', [UserController::class, 'show'])->name('users.show');
Route::get('/users/{user}/statistics', [UserController::class, 'statistics'])->name('users.statistics');

Route::get('/matches/{match}', [MatchController::class, 'show'])->name('matches.show');
Route::get('/matches/{match}/statistics', [MatchController::class, 'statistics'])->name('matches.statistics');
Route::get('/tournaments/{tournament}/matches', [MatchController::class, 'tournamentMatches'])->name('tournaments.matches');

