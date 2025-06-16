<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;

/**
 * Abstract base class for AST nodes
 * Provides common functionality
 */
abstract class AbstractNode implements Node
{
    /**
     * Accept a visitor
     *
     * @param NodeVisitor $visitor The visitor to accept
     * @return mixed The result of the visit
     */
    public function accept(NodeVisitor $visitor): mixed
    {
        // This method should be overridden by all subclasses
        // to call the appropriate visit method on the visitor
        throw new \Exception("Abstract method called");
    }
    
    /**
     * Convert to string for debugging
     *
     * @return string String representation
     */
    public function __toString(): string
    {
        return static::class;
    }
}
