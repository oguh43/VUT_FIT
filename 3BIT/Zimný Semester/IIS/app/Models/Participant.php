<?php

// Participant.php
// Filip Jenis (xjenisf00)

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;
use Illuminate\Database\Eloquent\Relations\MorphTo;

class Participant extends Model
{
    protected $fillable = [
        'participant_type',
        'participant_id',
        'tournament_id',
        'approved'
    ];

    public function tournament()
    {
        return $this->belongsTo(Tournament::class);
    }

    public function participant()
    {
        return $this->morphTo();
    }
}
