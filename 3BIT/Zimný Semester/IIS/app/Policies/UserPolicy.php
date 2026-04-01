<?php

// UserPolicy.php
// Hugo Bohácsek (xbohach00)

namespace App\Policies;

use App\Models\User;

class UserPolicy
{
    public function view(?User $user, User $targetUser): bool
    {
        return true;
    }

    public function update(User $user, User $targetUser): bool
    {
        return $user->id === $targetUser->id || $user->isAdmin();
    }

    public function delete(User $user, User $targetUser): bool
    {
        return $user->isAdmin();
    }

    public function viewSchedule(User $user, User $targetUser): bool
    {
        return $user->id === $targetUser->id || $user->isAdmin();
    }
}
