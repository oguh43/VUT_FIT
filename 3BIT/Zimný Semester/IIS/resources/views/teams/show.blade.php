{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'teams')

@section('content')

    <main id="team_detail_page" class="detail_page">
        <section>
            <h3>{{$team->name}}</h3>
            <div class="profile_img">
                <img src="{{asset($team->image)}}" alt="profile picture">
            </div>

            <div>
                <ul>
                    <li>
                        <p class="field_name">Team name:</p>
                        <p class="value_field">{{$team->name}}</p>
                    </li>
                    <li>
                        <p class="field_name">Manager's name:</p>
                        <p class="value_field"><a href="{{route('users.show', $team->manager)}}">{{$team->manager->name}}</a></p>
                    </li>
                    <li>
                        <p class="field_name">Members:</p>
                        <ul>
                            @forelse ($team->members as $member)
                                <li class="members_edit_button"><p><a href="{{route('users.show', $member)}}">{{$member->name}}</a></p>
                                @auth
                                    @if(auth()->id() === $team->manager_id || auth()->user()->isAdmin())
                                        <form method="POST" action="{{route('teams.members.remove', [$team, $member])}}">
                                            @csrf
                                            @method('DELETE')
                                            <button type="submit" class="remove_member">Remove</button>
                                        </form>
                                    @endif
                                @endauth
                                </li>
                            @empty
                                <li><p>No members</p></li>
                            @endforelse
                        </ul>
                    </li>
                    @auth
                    @if(auth()->id() === $team->manager_id || auth()->user()->isAdmin())
                    <li class="members_edit_button">
                        <form method="POST" action="{{route('teams.members.add', $team)}}">
                            @csrf
                            <select name="user_id">
                                @foreach($availableUsers as $user)
                                    <option value="{{$user->id}}">{{$user->name}}</option>
                                @endforeach
                            </select>
                            <button type="submit">Add member</button>
                        </form>
                    </li>
                    @endif
                    @endauth
                    <li>
                        <p class="field_name">Statistics:</p>
                        <ul class="value_field">
                            <li><p>Total tournaments: {{$stats['total_tournaments']}}</p></li>
                            <li><p>Active tournaments: {{$stats['active_tournaments']}}</p></li>
                            <li><p>Total matches: {{$stats['total_matches']}}</p></li>
                            <li><p>Wins: {{$stats['matches_won']}}</p></li>
                            <li><p>Losses: {{$stats['matches_lost']}}</p></li>
                            <li><p>Win rate: {{$stats['win_rate']}}%</p></li>
                            <li><p>Total score: {{$stats['total_score']}}</p></li>
                            <li><p>Total conceded: {{$stats['total_conceded']}}</p></li>
                            <li><p>Average score: {{$stats['average_score']}}</p></li>
                            <li><p>Member count: {{$stats['members_count']}}</p></li>
                        </ul>
                    </li>
                    <li>
                        <p class="field_name">Recent matches:</p>
                        <ul>
                            @forelse ($recentMatches as $match)
                                @php
                                    $p1 = optional(optional($match->participant1)->participant);
                                    $p2 = optional(optional($match->participant2)->participant);
                                @endphp

                                <li class="recent-match">
                                    <p class="match-info"><a href="{{route('tournaments.show', $$match->tournament)}}"><strong>{{$match->tournament->name}}</strong></a>: {{$p1->name ?? 'TBD'}} vs {{$p2->name ?? 'TBD'}} <i>(Score: {{$match->score1 ?? '-'}}:{{$match->score2 ?? '-'}})</i></p>
                                </li>
                            @empty
                                <li><p>No recent matches</p></li>
                            @endforelse
                        </ul>
                    </li>
                </ul>
            </div>
            @auth
            <div class="edit_buttons">
                @if($team->members->contains(auth()->user()->id))
                    <form method="POST" action="{{route('teams.leave', $team)}}">
                        @csrf
                        <button type="submit">Leave team</button>
                    </form>
                @endif
                @if (auth()->id() === $team->manager_id || auth()->user()->isAdmin())
                    <form method="GET" action="{{route('teams.edit', $team)}}" class="edit_team_form">
                        @csrf
                        <button type="submit">Edit</button>
                    </form>
                    <form method="POST" action="{{route('teams.transfer-ownership', $team)}}">
                        @csrf
                        <select name="new_manager_id" id="new_manager_id">
                            @foreach($team->members->where('id', '!=', $team->manager_id) as $member)
                                <option value="{{$member->id}}">{{$member->name}}</option>
                            @endforeach
                        </select>
                        <button type="submit">Transfer ownership</button>
                    </form>
                    <form method="POST" action="{{route('teams.destroy', $team)}}">
                        @csrf
                        @method('DELETE')
                        <button type="submit">Delete team</button>
                    </form>
                @endif
            </div>
            @endauth
        </section>
    </main>
@endsection
