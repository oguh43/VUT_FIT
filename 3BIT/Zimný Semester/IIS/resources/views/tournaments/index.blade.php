{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="tournaments_page" class="list_page">
        <section>
            <h3>Tournaments</h3>
            <form method="GET" action="">
                <input type="text" name="search" placeholder="Search tournaments..." value="{{request('search')}}">
                <button type="submit">Search</button>
            </form>

            <div class="list">
                @foreach ($tournaments as $tournament)
                    @auth
                        <div class="list_elem {{$tournament->manager_id == auth()->user()->id ? 'auth_user' :""}}">
                            <a href="{{route('tournaments.show', $tournament)}}">
                                <p>Name: {{$tournament->name}}</p>
                                <p>Type: {{$tournament->type}}</p>
                                <p>Participants: {{$tournament->participants->count()}}/{{$tournament->max_participants}}</p>
                                @if ($tournament->pricepool)
                                    <p>Prize pool: {{$tournament->pricepool}} €</p>
                                @endif
                            </a>
                        </div>
                    @endauth

                    @guest
                        <div class="list_elem">
                            <a href="{{route('tournaments.show', $tournament)}}">
                                <p>Name: {{$tournament->name}}</p>
                                <p>Type: {{$tournament->type}}</p>
                                <p>Participants: {{$tournament->participants->count()}}/{{$tournament->max_participants}}</p>
                                @if ($tournament->pricepool)
                                    <p>Prize pool: {{$tournament->pricepool}} €</p>
                                @endif
                            </a>
                        </div>
                    @endguest
                @endforeach
            </div>
            @auth
            <div class="edit_buttons">
                <a href="{{route('tournaments.create')}}">
                    <button type="button">Create Tournament</button>
                </a>
            </div>
            @endauth
        </section>
    </main>
@endsection
