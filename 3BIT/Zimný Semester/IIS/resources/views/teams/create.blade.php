{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'teams')

@section('content')

    <main id="team_create_page" class="edit_page">
        <section>
            <form action="{{route('teams.store')}}" method="POST" enctype="multipart/form-data">
                @csrf
                <h3>Create new team</h3>
                <div class="profile_img">
                    <input type="file" name="image">
                    @error('image')
                        <p class="error">{{$message}}</p>
                    @enderror
                </div>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Name:</p>
                            <input type="text" name="name" placeholder="team name">
                            @error('name')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                    </ul>
                </div>

                <div>
                    <input type="submit" value="Create team" name="save_button">
                    <a href="{{route('teams.index')}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>
@endsection
