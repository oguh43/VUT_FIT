<?php

// ParticipantPolicy.php
// Hugo Bohácsek (xbohach00)

namespace App\Policies;

use App\Models\Participant;
use App\Models\User;
use App\Models\Team;

class ParticipantPolicy
{
    public function cancel(User $user, Participant $participant): bool
    {
        // Individuálna účasť
        if ($participant->participant_type === User::class) {
            return $participant->participant_id === $user->id;
        }

        // Týmová účasť
        if ($participant->participant_type === Team::class) {
            $team = Team::find($participant->participant_id);
            return $team && $team->manager_id === $user->id;
        }

        return false;
    }
}
