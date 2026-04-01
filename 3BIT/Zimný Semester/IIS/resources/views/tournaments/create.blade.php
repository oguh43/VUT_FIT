{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="tournament_create_page" class="edit_page">
        <section>
            <form action="{{route('tournaments.store')}}" method="POST" enctype="multipart/form-data">
                @csrf
                <h3>Create new tournament</h3>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Name:</p>
                            <input type="text" name="name" placeholder="Tournament name">
                            @error('name')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Description:</p>
                            <textarea name="description" placeholder="Optional description"></textarea>
                            @error('description')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Prize Pool [€]:</p>
                            <input type="number" name="pricepool" placeholder="0.00" step="1.00">
                            @error('pricepool')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Type:</p>
                            <select name="type">
                                <option value="individual">Individual</option>
                                <option value="team">Team</option>
                            </select>
                            @error('type')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Max Participants:</p>
                            <input type="number" name="max_participants" min="2">
                            @error('max_participants')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Date:</p>
                            <input type="date" name="date">
                            @error('date')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                    </ul>
                </div>

                <div>
                    <input type="submit" value="Create Tournament" name="save_button">
                    <a href="{{route('tournaments.index')}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>
@endsection
