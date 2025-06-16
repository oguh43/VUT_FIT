<?php

namespace IPP\Student\AST\Node;

use IPP\Student\AST\Visitor\NodeVisitor;
use IPP\Student\Runtime\Value\BlockValue;
use IPP\Student\Runtime\Value\NilValue;
use IPP\Student\Runtime\SymbolTable;

/**
 * Represents a block of code in the AST
 */
class BlockNode extends AbstractNode
{
    private int $arity;
    /** @var array<int, array<string, mixed>> */
    private array $parameters = [];
    /** @var array<AssignNode> */
    private array $assigns = [];

    /**
     * Constructor
     *
     * @param \DOMElement $element XML element
     */
    public function __construct(\DOMElement $element)
    {
        $this->arity = (int)$element->getAttribute('arity');

        foreach ($element->childNodes as $child) {
            if ($child instanceof \DOMElement) {
                if ($child->tagName === 'parameter') {
                    $this->parameters[] = [
                        'order' => (int)$child->getAttribute('order'),
                        'name' => $child->getAttribute('name')
                    ];
                } elseif ($child->tagName === 'assign') {
                    $this->assigns[] = new AssignNode($child);
                }
            }
        }

        // Sort parameters and assigns by order
        usort($this->parameters, fn($a, $b) => $a['order'] <=> $b['order']);
        usort($this->assigns, fn($a, $b) => $a->getOrder() <=> $b->getOrder());
    }

    /**
     * Accept a visitor
     *
     * @param NodeVisitor $visitor The visitor to accept
     * @return mixed The result of the visit
     */
    public function accept(NodeVisitor $visitor): mixed
    {
        return $visitor->visitBlock($this);
    }

    /**
     * Get block arity (number of parameters)
     *
     * @return int Arity
     */
    public function getArity(): int
    {
        return $this->arity;
    }

    /**
     * Get parameters
     *
     * @return array<int, array<string, mixed>> Parameters
     */
    public function getParameters(): array
    {
        return $this->parameters;
    }

    /**
     * Get assigns
     *
     * @return array<AssignNode> Assigns
     */
    public function getAssigns(): array
    {
        return $this->assigns;
    }

    /**
     * Create a BlockValue from this BlockNode
     *
     * @param SymbolTable $symbolTable Symbol table for the block scope
     * @return BlockValue Block value
     */
    public function createBlockValue(SymbolTable $symbolTable): BlockValue
    {
        return new BlockValue($this, $symbolTable);
    }

    /**
     * Validate that parameter count matches arity
     *
     * @return bool True if valid
     */
    public function validateParameterCount(): bool
    {
        return count($this->parameters) === $this->arity;
    }

    /**
     * Find parameter by name
     * 
     * @param string $name Parameter name
     * @return array<string, mixed>|null Parameter or null if not found
     */
    public function findParameter(string $name): ?array
    {
        foreach ($this->parameters as $param) {
            if ($param['name'] === $name) {
                return $param;
            }
        }
        return null;
    }

    /**
     * Check if block has any assigns
     *
     * @return bool True if block has assigns
     */
    public function hasAssigns(): bool
    {
        return !empty($this->assigns);
    }

    /**
     * Get last assign (for return value calculation)
     *
     * @return AssignNode|null Last assign or null if none
     */
    public function getLastAssign(): ?AssignNode
    {
        if (empty($this->assigns)) {
            return null;
        }
        return $this->assigns[count($this->assigns) - 1];
    }

    /**
     * Helper to evaluate all statements and return the last result
     * This is useful for testing but normally the visitor would handle evaluation
     *
     * @param SymbolTable $symbolTable Symbol table
     * @param \IPP\Student\Runtime\ClassManagement\DefinedClasses $definedClasses Defined classes
     * @param \IPP\Core\StreamWriter $writer Output writer
     * @param \IPP\Core\Interface\InputReader $input Input reader
     * @return mixed Result of the last statement
     */
    public function evaluateBlock(
        SymbolTable $symbolTable,
        \IPP\Student\Runtime\ClassManagement\DefinedClasses $definedClasses,
        \IPP\Core\StreamWriter $writer,
        \IPP\Core\Interface\InputReader $input
    ): mixed {
        $visitor = new \IPP\Student\AST\Visitor\InterpreterVisitor(
            $symbolTable,
            $definedClasses,
            $writer,
            $input
        );

        $result = null;
        foreach ($this->assigns as $assign) {
            $result = $assign->accept($visitor);
        }

        return $result ?? NilValue::getInstance();
    }
}
