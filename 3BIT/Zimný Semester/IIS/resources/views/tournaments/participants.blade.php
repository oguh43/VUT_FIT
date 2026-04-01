{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="tournament_participants_page" class="detail_page">
        <section>
            <h3>{{$tournament->name}} participants</h3>
            <div>
                <ul>
                    <li>
                        <p class="field_name">Approved Participants:</p>
                        <ul>
                            @forelse($approved as $participant)
                                <li><p>{{$participant->participant->name}}</p></li>
                            @empty
                                <li><p>No approved participants</p></li>
                            @endforelse
                        </ul>
                    </li>
                    <li>
                        <p class="field_name">Pending Participants:</p>
                        <ul>
                            @forelse($pending as $participant)
                                <li class="members_edit_button"><p>{{$participant->participant->name}}</p>
                                @auth
                                    @if(auth()->id() === $tournament->manager_id || auth()->user()->isAdmin())
                                        <form method="POST" action="{{route('participants.approve', [$tournament, $participant])}}">
                                            @csrf
                                            <button type="submit">Approve</button>
                                        </form>
                                        <form method="POST" action="{{route('participants.reject', [$tournament, $participant])}}">
                                            @csrf
                                            @method('DELETE')
                                            <button type="submit">Reject</button>
                                        </form>
                                    @endif
                                @endauth
                                </li>
                            @empty
                                <li><p>No pending participants</p></li>
                            @endforelse
                        </ul>
                    </li>
                </ul>
            </div>
        </section>
    </main>

@endsection
