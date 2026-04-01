{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="match_edit_page" class="edit_page">
        <section>
            @php
                $p1Name = $match->participant1?->participant?->name ?? 'TBD';
                $p2Name = $match->participant2?->participant?->name ?? 'TBD';
                $hasSecond = !is_null($match->participant2_id);
            @endphp
            <form action="{{route('matches.update', $match)}}" method="POST">
                @csrf
                @method('PUT')
                <h3>{{$p1Name}} vs {{$p2Name}}</h3>
                <div>
                    <ul>
                        <li>
                            <p class="field_name">{{$p1Name}}</p>
                            <input type="number" name="score1" min="1" value="{{old('score1', $match->score1)}}">
                            @error('score1')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        @if ($hasSecond)
                            <li>
                                <p class="field_name">{{$p2Name}}</p>
                                <input type="number" name="score2" min="1" value="{{old('score2', $match->score2)}}">
                                @error('score2')
                                    <p class="error">{{$message}}</p>
                                @enderror
                            </li>
                        @else
                            <li>
                                <p class="field_name">Empty</p>
                                <p class="info">This player slot is empty -- {{$p1Name}} wins automatically.</p>
                                <input type="hidden" name="score2" value="0">
                            </li>
                        @endif

                    </ul>
                </div>

                <div>
                    <input type="submit" value="Save result" name="save_button">
                    <a href="{{route('tournaments.show', $match->tournament)}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>

@endsection
