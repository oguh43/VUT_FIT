<?php

// AdminMiddleware.php
// Filip Jenis (xjenisf00)

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class AdminMiddleware
{
    public function handle(Request $request, Closure $next): Response
    {
        if (!auth()->check() || !auth()->user()->isAdmin()) {
            abort(403, 'Nemáte oprávnenie pristupovať k tejto sekcii.');
        }

        return $next($request);
    }
}
