{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'teams')

@section('content')

    <main id="team_edit_page" class="edit_page">
        <section>
            <form action="{{route('teams.update', $team)}}" method="POST" enctype="multipart/form-data">
                @csrf
                <h3>{{$team->name}}</h3>
                <div class="profile_img">
                    <input type="file" name="image" value="{{asset($team->image)}}">
                    @error('image')
                        <p class="error">{{$message}}</p>
                    @enderror
                </div>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Name:</p>
                            <input type="text" name="name" value="{{$team->name}}">
                            @error('name')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                    </ul>
                </div>

                <div>
                    <input type="submit" value="Save changes" name="save_button">
                    <a href="{{route('teams.show', $team)}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>
@endsection
