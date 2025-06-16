<?php

namespace IPP\Student\Runtime\ClassManagement;

use IPP\Student\AST\Node\MethodNode;
use IPP\Student\Exception\OtherRuntimeException;

/**
 * Information about a user-defined class
 */
class UserClassInfo extends ClassInfo
{
    /** @var array<string, MethodNode> */
    private array $methodMap = [];

    /**
     * Constructor
     *
     * @param string $name Class name
     * @param string $parentName Parent class name
     * @param array<string> $methods Method selectors
     */
    public function __construct(string $name, string $parentName, array $methods = [])
    {
        parent::__construct($name, $parentName, $methods);
    }

    /**
     * Add a method implementation
     *
     * @param string $selector Method selector
     * @param MethodNode $method Method implementation
     * @return void
     */
    public function addMethodNode(string $selector, MethodNode $method): void
    {
        $this->addMethod($selector);
        $this->methodMap[$selector] = $method;
    }

    /**
     * Get a method implementation
     *
     * @param string $selector Method selector
     * @return MethodNode Method implementation
     * @throws OtherRuntimeException If the method is not found
     */
    public function getMethodNode(string $selector): MethodNode
    {
        if (!isset($this->methodMap[$selector])) {
            throw new OtherRuntimeException("Method {$selector} not found in class {$this->name}");
        }
        
        return $this->methodMap[$selector];
    }

    /**
     * Check if method implementation exists
     *
     * @param string $selector Method selector
     * @return bool True if implementation exists
     */
    public function hasMethodNode(string $selector): bool
    {
        return isset($this->methodMap[$selector]);
    }

    /**
     * Get all method implementations
     *
     * @return array<string, MethodNode> Method implementations
     */
    public function getAllMethodNodes(): array
    {
        return $this->methodMap;
    }
}
