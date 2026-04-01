{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'teams')

@section('content')

    <main id="teams_page" class="list_page">
        <section>
            <h3>Teams</h3>

            <form method="GET" action="/teams">
                <input type="text" name="search" placeholder="Search teams..." value="{{request('search')}}">
                <button type="submit">Search</button>
            </form>

            <div class="list">
                @foreach ($teams as $team)
                    @auth
                        <div class="list_elem {{$team->manager_id == auth()->user()->id ? 'auth_user' :""}}">
                            <a href="{{route('teams.show', $team)}}">
                                <p>Name: <span>{{$team->name}}</span></p>
                            </a>
                        </div>
                    @endauth

                    @guest
                        <div class="list_elem">
                            <a href="{{route('teams.show', $team)}}">
                                <p>Name: <span>{{$team->name}}</span></p>
                            </a>
                        </div>
                    @endguest
                @endforeach
            </div>
            @auth
            <div class="edit_buttons">
                <a href="{{route('teams.create')}}">
                    <button type="button">Create Team</button>
                </a>
            </div>
            @endauth
        </section>
    </main>
@endsection
