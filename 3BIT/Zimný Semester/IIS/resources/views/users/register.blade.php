{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'register')

@section('content')
    <main id="register_page" class="form_page">


        <section>
            <h3>Register</h3>
            <form method="POST" action="/register" class>
                @csrf
                <input type="text" name="name" placeholder="username" required value="{{old('name')}}">
                @error('name')
                    <p class="error">{{$message}}</p>
                @enderror

                <input type="email" name="email" placeholder="example@email.cz" required value="{{old('email')}}">
                @error('email')
                    <p class="error">{{$message}}</p>
                @enderror

                <input type="password" name="password" placeholder="password" required value="{{old('password')}}">
                @error('password')
                    <p class="error">{{$message}}</p>
                @enderror

                <input type="submit" value="Create account" name="btn">
            </form>

            <a href="/login">Already have an account? Sign in...</a>
        </section>
    </main>
@endsection
