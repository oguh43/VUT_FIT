{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="tournament_edit_page" class="edit_page">
        <section>
            <form action="{{route('tournaments.update', $tournament)}}" method="POST">
                @csrf
                @method('PUT')
                <h3>{{$tournament->name}}</h3>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Name:</p>
                            <input type="text" name="name" value="{{$tournament->name}}">
                            @error('name')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Description:</p>
                            <textarea name="description">{{$tournament->description}}</textarea>
                            @error('description')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Prize Pool [€]:</p>
                            <input type="number" name="pricepool" value="{{$tournament->pricepool}}" step="1.00">
                            @error('pricepool')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Type:</p>
                            <select name="type">
                                <option value="individual" {{$tournament->type === 'individual' ? 'selected' : ''}}>Individual</option>
                                <option value="team" {{$tournament->type === 'team' ? 'selected' : ''}}>Team</option>
                            </select>
                            @error('type')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Max Participants:</p>
                            <input type="number" name="max_participants" min="2" value="{{$tournament->max_participants}}">
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
                    <input type="submit" value="Save changes" name="save_button">
                    <a href="{{route('tournaments.show', $tournament)}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>
@endsection
