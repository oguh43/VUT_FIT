{{-- Eliška Křeménková (xkremee00) --}}
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    {{-- style --}}
    <link rel="stylesheet" href="{{asset('css/style.css')}}">
    <title>Tournaments</title>
</head>
<body>
    <header>
        <ul id="menu">
            {{-- <li><a href="/"><h2>Tournaments</h2></a></li> --}}

            <li><a href="/" class="{{(View::hasSection('page') && View::getSection('page') == 'home') ? 'active' : ''}}">Home</a></li>
            <li><a href="/tournaments" class="{{(View::hasSection('page') && View::getSection('page') == 'tournaments') ? 'active' : ''}}">Tournaments</a></li>
            <li><a href="/teams" class="{{(View::hasSection('page') && View::getSection('page') == 'teams') ? 'active' : ''}}">Teams</a></li>
            <li><a href="/users" class="{{(View::hasSection('page') && View::getSection('page') == 'users') ? 'active' : ''}}">Users</a></li>

            @auth
            <li class="logged_in">
                <form method="POST" action="{{route('logout')}}">
                    @csrf
                    <button type="submit">Log out</button>
                </form>
            </li>
            <li class="logged_in"><a href="{{route('users.show', auth()->user())}}">{{auth()->user()->name}}</a></li>
            @endauth

            @guest
            <li class="logged_out {{(View::hasSection('page') && View::getSection('page') == 'login') ? 'hide' : ''}}"><a href="/login">Log in</a></li>
            <li class="logged_out {{(View::hasSection('page') && View::getSection('page') == 'register') ? 'hide' : ''}}"><a href="/register">Register</a></li>
            @endguest

        </ul>
    </header>

    {{-- flash messages --}}
    @if(Session::has('error'))
        <div class="flash error">
            <p>{{session('error')}}</p>
        </div>
    @elseif(Session::has('message'))
        <div class="flash success">
            <p>{{session('message')}}</p>
        </div>
    @endif

    {{-- main content --}}
    @yield('content')

    <footer>
        <p>© 2025 Tournaments.</p>
    </footer>
</body>
</html>
