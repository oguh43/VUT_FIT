<?php

// 06_create_tournament_matches_table.php
// Filip Jenis (xjenisf00)

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('tournament_matches', function (Blueprint $table) {
            $table->id();
            $table->foreignId('tournament_id')->constrained('tournaments')->onDelete('cascade');
            $table->foreignId('participant1_id')->nullable()->constrained('participants')->onDelete('cascade');
            $table->foreignId('participant2_id')->nullable()->constrained('participants')->onDelete('cascade');
            $table->integer('score1')->nullable();
            $table->integer('score2')->nullable();
            $table->boolean('finished')->default(false);
            $table->unsignedBigInteger('winner_id')->nullable();
            $table->integer('index');
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('tournament_matches');
    }
};
