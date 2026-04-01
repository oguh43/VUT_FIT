<?php

// Team.php
// Filip Jenis (xjenisf00)

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsToMany;
use Illuminate\Database\Eloquent\Relations\MorphMany;

class Team extends Model
{
    use HasFactory;
    protected $fillable = [
        'name',
        'image',
        'manager_id'
    ];

    public function members()
    {
        return $this->belongsToMany(User::class, 'teammembers', 'team_id', 'user_id');
    }

    public function manager()
    {
        return $this->belongsTo(User::class, 'manager_id');
    }

    public function tournaments()
    {
        return $this->morphMany(Participant::class, 'participant');
    }
}
