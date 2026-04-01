{{-- Eliška Křeménková (xkremee00) --}}
@extends('layouts.layout')

@section('page', 'login')

@section('content')
    <main id="login_page" class="form_page">


        <section>
            <h3>Log in</h3>
            <form method="POST" action="/login">
                @csrf
                <input type="email" name="email" placeholder="example@email.cz" required value="{{old('email')}}">
                @error('email')
                    <p class="error">{{$message}}</p>
                @enderror

                <input type="password" name="password" placeholder="password" required value="{{old('password')}}">
                @error('password')
                    <p class="error">{{$message}}</p>
                @enderror

                <input type="submit" value="Log in" name="btn">
            </form>

            <a href="/register">Don't have an account yet? Register now...</a>
        </section>
    </main>
@endsection
