<?php

namespace IPP\Student\Runtime;

use IPP\Student\Exception\OtherRuntimeException;

/**
 * Symbol table for storing variables
 */
class SymbolTable
{
    /** @var array<string, mixed> */
    private array $variables = [];
    
    /** @var SymbolTable|null */
    private ?SymbolTable $parent;

    /**
     * Constructor
     *
     * @param SymbolTable|null $parent Parent symbol table for scope chaining
     */
    public function __construct(?SymbolTable $parent = null)
    {
        $this->parent = $parent;
    }

    /**
     * Define a new variable or update an existing one
     *
     * @param string $name Variable name
     * @param mixed $value Variable value
     * @return void
     */
    public function define(string $name, mixed $value): void
    {
        $this->variables[$name] = $value;
    }

    /**
     * Get a variable value
     *
     * @param string $name Variable name
     * @return mixed Variable value
     * @throws OtherRuntimeException If the variable is not defined
     */
    public function get(string $name): mixed
    {
        // Check if the variable exists in this scope
        if (array_key_exists($name, $this->variables)) {
            return $this->variables[$name];
        }

        // Check parent scope
        if ($this->parent !== null) {
            return $this->parent->get($name);
        }

        // Variable not found
        throw new OtherRuntimeException("Variable '{$name}' is undefined");
    }

    /**
     * Set the value of an existing variable
     *
     * @param string $name Variable name
     * @param mixed $value New value
     * @return void
     * @throws OtherRuntimeException If the variable is not defined
     */
    public function set(string $name, mixed $value): void
    {
        if (!array_key_exists($name, $this->variables)) {
            throw new OtherRuntimeException("Variable '{$name}' is undefined");
        }
        
        $this->variables[$name] = $value;
    }

    /**
     * Check if a variable exists in this scope or any parent scope
     *
     * @param string $name Variable name
     * @return bool True if the variable exists
     */
    public function has(string $name): bool
    {
        if (array_key_exists($name, $this->variables)) {
            return true;
        }

        return $this->parent?->has($name) ?? false;
    }

    /**
     * Check if a variable exists in this scope only
     *
     * @param string $name Variable name
     * @return bool True if the variable exists in this scope
     */
    public function hasInThisScope(string $name): bool
    {
        return array_key_exists($name, $this->variables);
    }

    /**
     * Get a copy of all variables in this scope
     *
     * @return array<string, mixed> All variables
     */
    public function getAllVariables(): array
    {
        return $this->variables;
    }

    /**
     * Create a child scope
     *
     * @return SymbolTable New symbol table with this as parent
     */
    public function createChildScope(): SymbolTable
    {
        return new SymbolTable($this);
    }
}
