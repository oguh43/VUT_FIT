{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'users')

@section('content')

    <main id="edit_password_page" class="edit_page">
        <section>
            <form action="{{route('users.update-password', $user)}}" method="POST">
                @csrf
                <h3>{{$user->name}}</h3>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Current password:</p>
                            <input type="password" name="current_password">
                            @error('current_password')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">New password:</p>
                            <input type="password" name="password">
                            @error('password')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Confirm password:</p>
                            <input type="password" name="password_confirmation">
                            @error('password_confirmation')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                    </ul>
                </div>

                <div>
                    <input type="submit" value="Save changes" name="save_button">
                    <a href="{{route('users.show', $user)}}" class="cancel_button">Cancel</a>
                </div>
            </form>
        </section>
    </main>
@endsection
