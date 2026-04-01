{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'users')

@section('content')

    <main id="users_detail_page" class="detail_page">
        <div class="tabs">
            <div class="tab-buttons">
                <button class="tab-button active" data-tab="details">Details</button>
                <button class="tab-button" data-tab="schedule">Schedule</button>
            </div>
            <div class="tab-content active" id="details">
                <section>
                    <h3>{{$user->name}}</h3>
                    <div class="profile_img">
                        <img src="{{asset($user->image)}}" alt="profile picture">
                    </div>

                    <div>
                        <ul>
                            <li>
                                <p class="field_name">Name:</p>
                                <p class="value_field">{{$user->name}}</p>
                            </li>
                            <li>
                                <p class="field_name">Email:</p>
                                <p class="value_field">{{$user->email}}</p>
                            </li>
                            <li>
                                <p class="field_name">Teams:</p>
                                <ul>
                                    @forelse ($user->teams as $team)
                                        <li><p><a href="{{route('teams.show', $team)}}">{{$team->name}}</a></p></li>
                                    @empty
                                        <li><p>No teams</p></li>
                                    @endforelse
                                </ul>
                            </li>
                            <li>
                                <p class="field_name">Created teams:</p>
                                <ul>
                                    @forelse ($user->createdTeams as $team)
                                        <li><p><a href="{{route('teams.show', $team)}}">{{$team->name}}</a></p></li>
                                    @empty
                                        <li><p>None</p></li>
                                    @endforelse
                                </ul>
                            </li>
                            <li>
                                <p class="field_name">Created tournaments:</p>
                                <ul>
                                    @forelse ($user->createdTournaments as $tournament)
                                        <li><p><a href="{{route('tournaments.show', $tournament)}}">{{$tournament->name}}</a></p></li>
                                    @empty
                                        <li><p>None</p></li>
                                    @endforelse
                                </ul>
                            </li>
                            <li>
                                <p class="field_name">Statistics:</p>
                                <ul>
                                    <li><p>Total tournaments: {{$stats['total_tournaments']}}</p></li>
                                    <li><p>Total matches: {{$stats['total_matches']}}</p></li>
                                    <li><p>Matches won: {{$stats['matches_won']}}</p></li>
                                    <li><p>Matches lost: {{$stats['matches_lost']}}</p></li>
                                    <li><p>Win rate: {{$stats['win_rate']}}%</p></li>
                                    <li><p>Teams joined: {{$stats['teams_count']}}</p></li>
                                    <li><p>Teams created: {{$stats['created_teams']}}</p></li>
                                    <li><p>Tournaments created: {{$stats['created_tournaments']}}</p></li>
                                </ul>
                            </li>
                            <li>
                                <p class="field_name">Recent matches:</p>
                                @if ($recentMatches->isEmpty())
                                    <p>No matches played yet.</p>
                                @else
                                <ul>
                                    @foreach ($recentMatches as $match)
                                        @php
                                            $p1 = optional(optional($match->participant1)->participant);
                                            $p2 = optional(optional($match->participant2)->participant);
                                        @endphp
                                        <li>
                                            <p><a href="{{route('tournaments.show', $match->tournament)}}">{{$match->tournament->name}}</a></p>
                                            <p class="value_field">
                                                {{$p1->name ?? 'Empty'}}
                                                {{!is_null($match->score1) ? '('.$match->score1.')' : ''}}
                                                vs
                                                {{$p2->name ?? 'Empty'}}
                                                {{!is_null($match->score2) ? '('.$match->score2.')' : ''}}
                                            </p>
                                        </li>
                                    @endforeach
                                </ul>
                                @endif
                            </li>
                        </ul>
                    </div>
                    @auth
                    <div class="edit_buttons">
                        @if (auth()->id() === $user->id || auth()->user()->isAdmin())
                        <form method="GET" action="{{route('users.edit', $user)}}" class="edit_profile_form">
                            @csrf
                            <button type="submit">Edit</button>
                        </form>
                        @endif

                        @if (auth()->id() === $user->id)
                        <form method="GET" action="{{route('users.edit-password', $user)}}" class="edit_password_form">
                            @csrf
                            <button type="submit">Change password</button>
                        </form>
                        @endif

                        @if (auth()->user()->isAdmin())
                        <form method="POST" action="{{route('admin.users.change-role', $user)}}">
                            @csrf
                            <select name="role">
                                <option value="user" {{$user->role === 'user' ? 'selected' : ''}}>User</option>
                                <option value="admin" {{$user->role === 'admin' ? 'selected' : ''}}>Admin</option>
                            </select>
                            <button type="submit">Edit role</button>
                        </form>
                        <form method="POST" action="{{route('admin.users.destroy', $user)}}">
                            @csrf
                            @method('DELETE')
                            <button type="submit">Delete user</button>
                        </form>
                        @endif
                    </div>
                    @endauth
                </section>
            </div>
            <div class="tab-content" id="schedule">
                <section id="user_schedule">
                    <h3>Schedule</h3>
                    <div class="user_schedule_wrapper">
                        <div class="user_schedule_block">
                            <h4>Upcoming matches</h4>
                            @if ($upcomingMatches->isEmpty())
                                <p>No upcoming matches.</p>
                            @else
                                <ul>
                                    @foreach ($upcomingMatches as $match)
                                        @php
                                            $p1 = optional(optional($match->participant1)->participant);
                                            $p2 = optional(optional($match->participant2)->participant);
                                        @endphp
                                        <li class="schedule_match">
                                            <p class="field_name">
                                                <a href="{{route('tournaments.show', $match->tournament)}}">{{$match->tournament->name}}</a>
                                            </p>
                                            <p class="value_field">{{$p1->name ?? 'Empty'}} vs {{$p2->name ?? 'Empty'}}</p>
                                            <p class="value_field">Date: {{$match->tournament->date}}</p>
                                        </li>
                                    @endforeach
                                </ul>
                            @endif
                        </div>

                        <div class="user_schedule_block">
                            <h4>Past matches</h4>
                            @if ($pastMatches->isEmpty())
                                <p>No past matches.</p>
                            @else
                                <ul>
                                    @foreach ($pastMatches as $match)
                                        @php
                                            $p1 = optional(optional($match->participant1)->participant);
                                            $p2 = optional(optional($match->participant2)->participant);
                                        @endphp
                                        <li class="schedule_match">
                                            <p class="field_name">
                                                <a href="{{route('tournaments.show', $match->tournament)}}">{{$match->tournament->name}}</a>
                                            </p>
                                            <p class="value_field">
                                                {{$p1->name ?? 'Empty'}}
                                                {{!is_null($match->score1) ? '('.$match->score1.')' : ''}}
                                                vs
                                                {{$p2->name ?? 'Empty'}}
                                                {{!is_null($match->score2) ? '('.$match->score2.')' : ''}}
                                            </p>
                                            <p class="value_field">Status: {{$match->finished ? 'Finished' : 'Scheduled'}}</p>
                                        </li>
                                    @endforeach
                                </ul>
                            @endif
                        </div>
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
                if (hash && ['details', 'schedule'].includes(hash)) {
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
