<?php

// DatabaseSeeder.php
// Filip Jenis (xjenisf00)
// Hugo Bohácsek (xbohach00)

namespace Database\Seeders;

use App\Models\Participant;
use App\Models\Team;
use App\Models\User;
use App\Models\Tournament;
use App\Models\TournamentMatch;
use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;

class DatabaseSeeder extends Seeder
{
    use WithoutModelEvents;

    /**
     * Seed the application's database.
     */
    public function run(): void
    {
        User::factory(20)->create();

        $admin = User::create([
            'name' => 'Admin Adminovský',
            'email' => 'admin@testing.local',
            'password' => bcrypt('password123'),
            'role' => 'admin'
        ]);

        $testUsers = [
            [
                'name' => 'Jan Novák',
                'email' => 'novak@email.cz',
                'password' => bcrypt('password')
            ],
            [
                'name' => 'Petra Svobodová',
                'email' => 'svobodova@email.cz',
                'password' => bcrypt('password')
            ],
            [
                'name' => 'Martin Dvořák',
                'email' => 'dvorak@email.cz',
                'password' => bcrypt('password')
            ],
            [
                'name' => 'Eva Horáková',
                'email' => 'horakova@email.cz',
                'password' => bcrypt('password')
            ],
            [
                'name' => 'Tomáš Procházka',
                'email' => 'prochazka@gmail.com',
                'password' => bcrypt('password')
            ],
            [
                'name' => 'Jozef Mrkva',
                'email' => 'jozef@mrkva.sk',
                'password' => bcrypt('password')
            ]
        ];

        $users = collect();
        foreach ($testUsers as $user) {
            $users->push(User::create($user));
        }

        $team1 = Team::create([
            'name' => 'Team A',
            'manager_id' => $users[0]->id
        ]);
        $team2 = Team::create([
            'name' => 'Team B',
            'manager_id' => $users[1]->id
        ]);

        Team::factory(10)->create()->each(function ($team) {
            $manager = User::find($team->manager_id);
            $members = User::where('id', '!=', $team->manager_id)->take(rand(1,5))->get();
            $members->push($manager);
            $team->members()->attach($members);
        });

        $team1->members()->attach([$users[0]->id, $users[2]->id, $users[4]->id]);
        $team2->members()->attach([$users[1]->id, $users[3]->id, $users[5]->id]);

        $tournament1 = Tournament::create([
            'name' => 'Tournament ABC',
            'description' => 'Description 1',
            'pricepool' => 1000,
            'type' => 'team',
            'max_participants' => 2,
            'date' => now()->addDays(5),
            'manager_id' => $users[0]->id
        ]);

        Tournament::factory(10)->create()->each(function ($tournament){
            $validSizes = [2, 4, 8, 16, 32, 64];
            $possibleSizes = array_filter($validSizes, fn($size) => $size <= $tournament->max_participants);

            $approvedCount = $possibleSizes[array_rand($possibleSizes)];

            $pendingCount = rand(0, 3);

            if ($tournament->type === 'individual') {
                $allUsers = User::inRandomOrder()->take($approvedCount + $pendingCount)->get();
                foreach ($allUsers->take($approvedCount) as $user) {
                    Participant::create([
                        'tournament_id' => $tournament->id,
                        'participant_type' => User::class,
                        'participant_id' => $user->id,
                        'approved' => true
                    ]);
                }

                foreach ($allUsers->slice($approvedCount)->take($pendingCount) as $user) {
                    Participant::create([
                        'tournament_id' => $tournament->id,
                        'participant_type' => User::class,
                        'participant_id' => $user->id,
                        'approved' => false
                    ]);
                }
            }else{
                $allTeams = Team::inRandomOrder()->take($approvedCount + $pendingCount)->get();

                foreach ($allTeams->take($approvedCount) as $team) {
                    Participant::create([
                        'tournament_id' => $tournament->id,
                        'participant_type' => Team::class,
                        'participant_id' => $team->id,
                        'approved' => true
                    ]);
                }

                foreach ($allTeams->slice($approvedCount)->take($pendingCount) as $team) {
                    Participant::create([
                        'tournament_id' => $tournament->id,
                        'participant_type' => Team::class,
                        'participant_id' => $team->id,
                        'approved' => false
                    ]);
                }
            }
            $this->createMatches($tournament);
        });
    }

    protected function createMatches(Tournament $tournament)
    {
        if (!$tournament->approved){
            return;
        }
        $participants = $tournament->participants()->where('approved', true)->get();
        $participantCount = $participants->count();

        if ($participantCount < 2) {
            return;
        }

        $matchIndex = 0;
        $currentRoundParticipants = $participants->values();

        while ($currentRoundParticipants->count() > 1){
            $nextRoundParticipants = collect();
            foreach ($currentRoundParticipants->chunk(2) as $roundPair){

                $participant1 = $roundPair->first();
                $participant2 = $roundPair->last();

                $score1 = rand(0, 10);
                $score2 = rand(0, 10);

                if ($score1 == $score2){
                    $score1++;
                }

                $winner= $score1 > $score2 ? $participant1 : $participant2;
                $nextRoundParticipants->push($winner);

                TournamentMatch::create([
                    'tournament_id' => $tournament->id,
                    'participant1_id' => $participant1->id,
                    'participant2_id' => $participant2->id,
                    'index' => $matchIndex++,
                    'finished' => true,
                    'score1' => $score1,
                    'score2' => $score2,
                    'winner_id' => $winner->id
                ]);
            }
            $currentRoundParticipants = $nextRoundParticipants;
        }
    }
}
