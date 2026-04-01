{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'users')

@section('content')

    <main id="users_page" class="list_page">
        <section>
            <h3>Users</h3>
            <form method="GET" action="/users">
                <input type="text" name="search" placeholder="Search users..." value="{{request('search')}}">
                <button type="submit">Search</button>
            </form>

            <div class="list">
                @foreach ($users as $user)

                    @auth
                        <div class="list_elem {{$user->id == auth()->user()->id ? 'auth_user' :""}}">
                            <a href="{{route('users.show', $user)}}">
                                <p>Name: {{$user->name}}</p>
                                <p>Email: {{$user->email}}</p>
                            </a>
                        </div>
                    @endauth

                    @guest
                        <div class="list_elem">
                            <a href="{{route('users.show', $user)}}">
                                <p>Name: {{$user->name}}</p>
                                <p>Email: {{$user->email}}</p>
                            </a>
                        </div>
                    @endguest

                @endforeach
            </div>
        </section>
    </main>
@endsection
