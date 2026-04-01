<?php

// SessionTimeoutMiddleware.php
// Filip Jenis (xjenisf00)

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use Symfony\Component\HttpFoundation\Response;

class SessionTimeoutMiddleware
{
    /**
     * Handle an incoming request.
     *
     * @param  \Closure(\Illuminate\Http\Request): (\Symfony\Component\HttpFoundation\Response)  $next
     */
    private int $timeout = 15;

    public function handle(Request $request, Closure $next): Response
    {

        if (Auth::check()){
            $lastActivity = session('last_activity');

            if ($lastActivity) {

                if (abs(now()->diffInMinutes($lastActivity)) > $this->timeout) {
                    Auth::logout();
                    $request->session()->invalidate();
                    $request->session()->regenerateToken();

                    return redirect()->route('login')->with('error', 'Session timed out. Please log in again.');
                }
            }

            session(['last_activity' => now()]);
        }

        return $next($request);
    }
}
