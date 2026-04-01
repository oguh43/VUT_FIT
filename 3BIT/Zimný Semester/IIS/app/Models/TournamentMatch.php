<?php

// TournamentMatch.php
// Filip Jenis (xjenisf00)

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;
use Illuminate\Database\Eloquent\Relations\MorphTo;

class TournamentMatch extends Model
{
    protected $fillable = [
        'tournament_id',
        'participant1_id',
        'participant2_id',
        'score1',
        'score2',
        'finished',
        'winner_id',
        'index'
    ];

    public function tournament()
    {
        return $this->belongsTo(Tournament::class, 'tournament_id');
    }

    public function participant1()
    {
        return $this->belongsTo(Participant::class, 'participant1_id');
    }
    public function participant2()
    {
        return $this->belongsTo(Participant::class, 'participant2_id');
    }
    public function winner()
    {
        return $this->belongsTo(Participant::class, 'winner_id');
    }
}
