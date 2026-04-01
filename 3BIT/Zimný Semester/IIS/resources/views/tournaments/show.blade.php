{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'tournaments')

@section('content')

    <main id="tournament_detail_page" class="detail_page">
        <div class="tabs">
            <div class="tab-buttons">
                <button class="tab-button active" data-tab="details">Details</button>
                <button class="tab-button" data-tab="participants">Participants</button>
                <button class="tab-button" data-tab="bracket">Bracket</button>
            </div>

            <div class="tab-content active" id="details">
                <section>
                    <h3>{{$tournament->name}}</h3>

                    <div>
                        <ul>
                            <li>
                                <p class="field_name">Tournament name:</p>
                                <p class="value_field">{{$tournament->name}}</p>
                            </li>
                            <li>
                                <p class="field_name">Creator's name:</p>
                                <p class="value_field"><a href="{{route('users.show', $tournament->creator)}}">{{$tournament->creator->name}}</a></p>
                            </li>
                            <li>
                                <p class="field_name">Description:</p>
                                @if ($tournament->description)
                                    <p class="value_field">{{$tournament->description}}</p>
                                @else
                                    <p class="value_field">none</p>
                                @endif
                            </li>
                            <li>
                                <p class="field_name">Price pool:</p>
                                @if ($tournament->pricepool)
                                    <p class="value_field">{{$tournament->pricepool}} €</p>
                                @else
                                    <p class="value_field">none</p>
                                @endif
                            </li>
                            <li>
                                <p class="field_name">Type:</p>
                                <p class="value_field">{{$tournament->type}}</p>
                            </li>
                            <li>
                                <p class="field_name">Date:</p>
                                <p class="value_field">{{$tournament->date}}</p>
                            </li>
                            <li>
                                <p class="field_name">Approved:</p>
                                @if ($tournament->approved)
                                    <p class="value_field">yes</p>
                                @else
                                    <p class="value_field">no</p>
                                @endif
                            </li>
                            <li>
                                <p class="field_name">Statistics:</p>
                                <ul class="value_field">
                                    <li><p>Total participants: {{$stats['total_participants']}}</p></li>
                                    <li><p>Pending participants: {{$stats['pending_participants']}}</p></li>
                                    <li><p>Total matches: {{$stats['total_matches']}}</p></li>
                                    <li><p>Completed matches: {{$stats['completed_matches']}}</p></li>
                                </ul>
                            </li>
                        </ul>
                    </div>
                    <div class="edit_buttons">
                    @auth
                        @if(auth()->user()->isAdmin() && !$tournament->approved)
                            <form method="POST" action="{{route('admin.tournaments.approve', $tournament)}}">
                                @csrf
                                <button type="submit">Approve</button>
                            </form>
                            <form method="POST" action="{{route('admin.tournaments.reject', $tournament)}}">
                                @csrf
                                <button type="submit">Reject</button>
                            </form>
                        @endif

                        @if (auth()->id() === $tournament->manager_id || auth()->user()->isAdmin())
                            @if($tournament->approved && $tournament->matches->isEmpty())
                                <form method="POST" action="{{route('tournaments.start', $tournament)}}" id="start_button">
                                    @csrf
                                    <button type="submit">Start Tournament</button>
                                </form>
                            @endif
                            <form method="GET" action="{{route('tournaments.edit', $tournament)}}" class="edit_tournament_form">
                                @csrf
                                <button type="submit">Edit</button>
                            </form>
                            <form method="POST" action="{{route('tournaments.destroy', $tournament)}}">
                                @csrf
                                @method('DELETE')
                                <button type="submit">Delete tournament</button>
                            </form>
                        @endif

                        @if ($tournament->type === 'individual' && $tournament->matches->isEmpty())
                            <form method="POST" action="{{route('participants.storeIndividual', $tournament)}}">
                                @csrf
                                <button type="submit">Register</button>
                            </form>
                        @else
                            @if ($userTeams && $userTeams->isNotEmpty() && $tournament->matches->isEmpty())
                                <form method="POST" action="{{route('participants.storeTeam', $tournament)}}">
                                    @csrf
                                    <select name="team_id" id="team_id">
                                        @foreach($userTeams as $team)
                                            <option value="{{$team->id}}">{{$team->name}}</option>
                                        @endforeach
                                    </select>
                                    <button type="submit">Register Team</button>
                                </form>
                            @endif
                        @endif
                    @endauth
                    </div>
                </section>
            </div>

            <div class="tab-content" id="participants">
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
                                        @if ($tournament->type === 'individual')
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
                                        @else
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
                                        @endif
                                    @empty
                                        <li><p>No pending participants</p></li>
                                    @endforelse
                                </ul>
                            </li>
                        </ul>
                    </div>
                </section>
            </div>

            <div class="tab-content" id="bracket">
                <section class="detail_page" id="tournament_bracket">
                    <div class="bracket">
                        @if (empty($bracket))
                            <p>No matches have been generated for this tournament yet.</p>
                        @else
                            <div class="bracket-inner">
                            @php
                                $totalRounds = count($bracket);
                            @endphp
                            @foreach ($bracket as $roundNumber => $matches)
                                @php
                                    $roundsFromEnd = $totalRounds - $roundNumber + 1;

                                    if ($roundsFromEnd === 1) {
                                        $roundTitle = 'Final';
                                    } elseif ($roundsFromEnd === 2) {
                                        $roundTitle = 'Semifinal';
                                    } elseif ($roundsFromEnd === 3) {
                                        $roundTitle = 'Quarterfinal';
                                    } elseif ($roundsFromEnd === 4) {
                                        $roundTitle = 'Round of 16';
                                    } else {
                                        $roundTitle = 'Round ' . $roundNumber;
                                    }
                                @endphp

                                <div class="bracket-round">
                                    <h4>{{$roundTitle}}</h4>

                                    @forelse ($matches as $match)
                                        @php
                                            $p1 = $match->participant1 && $match->participant1->participant
                                                ? $match->participant1->participant
                                                : null;
                                            $p2 = $match->participant2 && $match->participant2->participant
                                                ? $match->participant2->participant
                                                : null;
                                        @endphp

                                        {{-- Skip completely empty matches (no players and not finished) --}}
                                        @if (is_null($p1) && is_null($p2) && !$match->finished)
                                            @continue
                                        @endif
                                        <div class="bracket-match">
                                            <div class="match-meta">
                                                <span class="match-index">Match #{{$match->index}}</span>
                                                @if ($match->finished)
                                                    <span class="match-status">Finished</span>
                                                @else
                                                    <span class="match-status">Scheduled</span>
                                                @endif
                                            </div>

                                            <div class="match-participant">
                                                {{$match->participant1 && $match->participant1->participant
                                                    ? $match->participant1->participant->name
                                                    : '---'}}

                                                @if (!is_null($match->score1) || !is_null($match->score2))
                                                    <span class="match-score">
                                                        {{$match->score1 ?? '-'}}
                                                    </span>
                                                @endif
                                            </div>

                                            <div class="match-participant">
                                                {{$match->participant2 && $match->participant2->participant
                                                    ? $match->participant2->participant->name
                                                    : '---'}}

                                                @if (!is_null($match->score1) || !is_null($match->score2))
                                                    <span class="match-score">
                                                        {{$match->score2 ?? '-'}}
                                                    </span>
                                                @endif
                                            </div>

                                            @can('update', $tournament)
                                                <div class="match-actions">
                                                    @if (!$match->finished)
                                                        <a href="{{route('matches.edit', $match)}}">
                                                            Evaluate match
                                                        </a>
                                                    @endif
                                                </div>
                                            @endcan
                                        </div>
                                    @empty
                                        <p>No matches in this round yet.</p>
                                    @endforelse
                                </div>
                            @endforeach
                            </div>
                        @endif
                    </div>
                </section>
            </div>
        </div>
        <script>
            function activateTab(tabName) {
                document.querySelectorAll('.tab-button').forEach(b => b.classList.remove('active'));
                document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));

                const targetButton = document.querySelector(`.tab-button[data-tab="${tabName}"]`);
                const targetContent = document.getElementById(tabName);

                if (targetButton && targetContent) {
                    targetButton.classList.add('active');
                    targetContent.classList.add('active');
                }
            }

            window.addEventListener('DOMContentLoaded', () => {
                const hash = window.location.hash.substring(1);
                if (hash && ['details', 'participants', 'bracket'].includes(hash)) {
                    activateTab(hash);
                }
            });

            document.querySelectorAll('.tab-button').forEach(button => {
                button.addEventListener('click', () => {
                    const tab = button.dataset.tab;
                    activateTab(tab);
                    window.location.hash = tab;
                });
            });
        </script>
    </main>
@endsection
