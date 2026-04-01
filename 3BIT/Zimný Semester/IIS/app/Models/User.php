<?php

// User.php
// Filip Jenis (xjenisf00)

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Foundation\Auth\User as Authenticatable;
use Illuminate\Notifications\Notifiable;
use Illuminate\Database\Eloquent\Relations\BelongsToMany;
use Illuminate\Database\Eloquent\Relations\HasMany;
use Illuminate\Database\Eloquent\Relations\MorphMany;

class User extends Authenticatable
{
    use HasFactory;
    protected $fillable = [
        'name',
        'email',
        'password',
        'role',
        'image'
    ];

    protected $hidden = [
        'password',
        'remember_token'
    ];

    public function isAdmin(): bool
    {
        return $this->role === 'admin';
    }

    public function createdTournaments()
    {
        return $this->hasMany(Tournament::class, 'manager_id');
    }

    public function tournaments()
    {
        return $this->morphMany(Participant::class, 'participant');
    }

    public function createdTeams()
    {
        return $this->hasMany(Team::class, 'manager_id');
    }

    public function teams()
    {
        return $this->belongsToMany(Team::class, 'teammembers', 'user_id', 'team_id');
    }
}
