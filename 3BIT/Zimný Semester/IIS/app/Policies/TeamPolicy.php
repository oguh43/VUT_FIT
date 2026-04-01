<?php

// TeamPolicy.php
// Hugo Bohácsek (xbohach00)

namespace App\Policies;

use App\Models\Team;
use App\Models\User;

class TeamPolicy
{
    public function view(?User $user, Team $team): bool
    {
        return true;
    }

    public function create(User $user): bool
    {
        return true;
    }

    public function update(User $user, Team $team): bool
    {
        return $user->id === $team->manager_id || $user->isAdmin();
    }

    public function delete(User $user, Team $team): bool
    {
        return $user->id === $team->manager_id || $user->isAdmin();
    }
}
