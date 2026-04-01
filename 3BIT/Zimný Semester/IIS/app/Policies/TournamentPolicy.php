<?php

// TournamentPolicy.php
// Hugo Bohácsek (xbohach00)

namespace App\Policies;

use App\Models\Tournament;
use App\Models\User;

class TournamentPolicy
{
    public function view(?User $user, Tournament $tournament): bool
    {
        return true; // Turnaje môžu vidieť všetci
    }

    public function create(User $user): bool
    {
        return true; // Každý prihlásený môže vytvoriť turnaj
    }

    public function update(User $user, Tournament $tournament): bool
    {
        // Správca turnaja alebo admin
        return $user->id === $tournament->manager_id || $user->isAdmin();
    }

    public function delete(User $user, Tournament $tournament): bool
    {
        return $user->id === $tournament->manager_id || $user->isAdmin();
    }

    public function approve(User $user, Tournament $tournament): bool
    {
        return $user->isAdmin(); // Iba admin môže schvaľovať
    }
}
