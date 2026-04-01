<?php

// TournamentFactory.php
// Filip Jenis (xjenisf00)

namespace Database\Factories;

use Illuminate\Database\Eloquent\Factories\Factory;

/**
 * @extends \Illuminate\Database\Eloquent\Factories\Factory<\App\Models\Tournament>
 */
class TournamentFactory extends Factory
{
    /**
     * Define the model's default state.
     *
     * @return array<string, mixed>
     */
    public function definition(): array
    {
        return [
            'name' => 'Tournament '.fake()->unique()->randomNumber(3),
            'description' => fake()->paragraph(),
            'date' => fake()->dateTimeBetween('+1 days', '+1 months'),
            'pricepool' => fake()->randomFloat(2, 100, 10000),
            'approved' => rand(0, 1),
            'max_participants' => rand(2, round((20 / 2), 0, PHP_ROUND_HALF_DOWN)) * 2,
            'type' => fake()->randomElement(['team', 'individual']),
            'manager_id' => rand(1, 20)
        ];
    }
}
