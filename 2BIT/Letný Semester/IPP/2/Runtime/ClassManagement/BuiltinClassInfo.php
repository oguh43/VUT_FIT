<?php

namespace IPP\Student\Runtime\ClassManagement;

/**
 * Information about a built-in class
 */
class BuiltinClassInfo extends ClassInfo
{
    /**
     * Constructor
     *
     * @param string $name Class name
     * @param array<string> $methods Method selectors
     */
    public function __construct(string $name, array $methods = [])
    {
        // Special handling for Object to avoid infinite loops
        $parentName = ($name === 'Object') ? null : 'Object';
        parent::__construct($name, $parentName, $methods);
    }

    /**
     * Check if this is a singleton class (True, False, Nil)
     *
     * @return bool True if this is a singleton class
     */
    public function isSingleton(): bool
    {
        return in_array($this->name, ['True', 'False', 'Nil'], true);
    }

    /**
     * Check if this class is a value class (Integer, String, etc.)
     *
     * @return bool True if this is a value class
     */
    public function isValueClass(): bool
    {
        return in_array($this->name, ['Integer', 'String', 'Block'], true);
    }
}
