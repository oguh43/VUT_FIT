{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'users')

@section('content')

    <main id="edit_user_page" class="edit_page">
        <section>
            <form action="{{route('users.update', $user)}}" method="POST" enctype="multipart/form-data">
                @csrf
                <h3>{{$user->name}}</h3>
                <div class="profile_img">
                    <input type="file" name="image" value="{{asset($user->image)}}">
                    @error('image')
                        <p class="error">{{$message}}</p>
                    @enderror
                </div>

                <div>
                    <ul>
                        <li>
                            <p class="field_name">Name:</p>
                            <input type="text" name="name" value="{{$user->name}}">
                            @error('name')
                                <p class="error">{{$message}}</p>
                            @enderror
                        </li>
                        <li>
                            <p class="field_name">Email:</p>
                            <input type="email" name="email" value="{{$user->email}}">
                            @error('email')
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
