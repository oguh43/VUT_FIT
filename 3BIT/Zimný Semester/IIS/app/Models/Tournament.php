<?php

// Tournament.php
// Filip Jenis (xjenisf00)

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\HasMany;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class Tournament extends Model
{
    use HasFactory;
    protected $fillable = [
        'name',
        'description',
        'pricepool',
        'type',
        'max_participants',
        'date',
        'manager_id',
        'approved'
    ];

    public function isIndividual(): bool
    {
        return $this->type === 'individual';
    }

    public function participants()
    {
        return $this->hasMany(Participant::class);
    }

    public function matches()
    {
        return $this->hasMany(TournamentMatch::class);
    }

    public function creator()
    {
        return $this->belongsTo(User::class, 'manager_id');
    }
}
