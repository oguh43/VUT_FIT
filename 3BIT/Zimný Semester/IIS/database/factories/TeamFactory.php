<?php

// TeamFactory.php
// Filip Jenis (xjenisf00)

namespace Database\Factories;

use Illuminate\Database\Eloquent\Factories\Factory;

/**
 * @extends \Illuminate\Database\Eloquent\Factories\Factory<\App\Models\Team>
 */
class TeamFactory extends Factory
{
    /**
     * Define the model's default state.
     *
     * @return array<string, mixed>
     */
    public function definition(): array
    {
        return [
            'name' => 'Team '.fake()->unique()->randomNumber(3),
            'manager_id' => rand(1, 20)
        ];
    }
}
