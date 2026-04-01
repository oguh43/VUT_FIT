<?php

// 05_create_participants_table.php
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
        Schema::create('participants', function (Blueprint $table) {
            $table->id();
            $table->foreignId('tournament_id')->constrained('tournaments')->onDelete('cascade');
            $table->morphs('participant');
            $table->boolean('approved')->default(false);
            $table->timestamps();

            $table->unique(['tournament_id', 'participant_id', 'participant_type'], 'tournament_participant_unique');

            $table->index('tournament_id');
            $table->index(['participant_id', 'participant_type']);
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('participants');
    }
};
