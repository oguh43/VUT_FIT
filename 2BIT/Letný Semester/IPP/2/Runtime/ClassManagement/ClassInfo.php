<?php

namespace IPP\Student\Runtime\ClassManagement;

/**
 * Base class for SOL25 class information
 */
abstract class ClassInfo
{
    /**
     * Constructor
     *
     * @param string $name Class name
     * @param string|null $parentName Parent class name
     * @param array<string> $methods Available methods
     */
    public function __construct(
        protected string $name,
        protected ?string $parentName,
        /** @var array<string> */
        protected array $methods = []
    ) {
    }

    /**
     * Get class name
     *
     * @return string Class name
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Get parent class name
     *
     * @return string|null Parent class name or null for Object
     */
    public function getParentName(): ?string
    {
        return $this->parentName;
    }

    /**
     * Get available methods
     *
     * @return array<string> Method selectors
     */
    public function getMethods(): array
    {
        return $this->methods;
    }

    /**
     * Check if this class has a method
     *
     * @param string $selector Method selector
     * @return boolean True if the method exists
     */
    public function hasMethod(string $selector): bool
    {
        return in_array($selector, $this->methods, true);
    }

    /**
     * Add a method to this class
     * 
     * @param string $selector Method selector
     * @return void
     */
    public function addMethod(string $selector): void
    {
        if (!$this->hasMethod($selector)) {
            $this->methods[] = $selector;
        }
    }
}
