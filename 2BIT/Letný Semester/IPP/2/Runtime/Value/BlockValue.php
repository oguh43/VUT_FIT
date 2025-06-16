<?php

namespace IPP\Student\Runtime\Value;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Exception\InvalidArgValueException;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;
use IPP\Student\Runtime\SymbolTable;
use IPP\Student\AST\Node\BlockNode;
use IPP\Student\AST\Visitor\InterpreterVisitor;

/**
 * Represents a SOL25 Block object at runtime
 * Blocks are executable code objects
 */
class BlockValue extends AbstractValue
{
    /**
     * Constructor
     *
     * @param BlockNode $blockNode AST block node
     * @param SymbolTable $lexicalScope Lexical scope when the block was created
     */
    public function __construct(
        private BlockNode $blockNode,
        private SymbolTable $lexicalScope
    ) {
    }

    /**
     * Get class name
     *
     * @return string
     */
    public function getClassName(): string
    {
        return 'Block';
    }

    /**
     * Get raw value (always null for blocks)
     *
     * @return mixed
     */
    public function getValue(): mixed
    {
        return null;
    }

    /**
     * Get the arity (number of parameters)
     *
     * @return int
     */
    public function getArity(): int
    {
        return $this->blockNode->getArity();
    }

    /**
     * Get the parameters
     *
     * @return array<int, array<string, mixed>>
     */
    public function getParameters(): array
    {
        return $this->blockNode->getParameters();
    }

    /**
     * Get the AST block node
     *
     * @return BlockNode
     */
    public function getBlockNode(): BlockNode
    {
        return $this->blockNode;
    }

    /**
     * Get the lexical scope
     *
     * @return SymbolTable
     */
    public function getLexicalScope(): SymbolTable
    {
        return $this->lexicalScope;
    }

    /**
     * Handle Block-specific messages
     *
     * @param string $selector Message selector
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return mixed Result of the message
     */
    protected function handleSpecificMessage(
        string $selector,
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        // Handle value/value:/value:value: etc. based on arity
        if ($this->isValueMessage($selector)) {
            return $this->executeBlock($selector, $args, $definedClasses, $writer, $input);
        }

        return match ($selector) {
            'whileTrue:' => $this->whileTrue($args, $definedClasses, $writer, $input),
            'isBlock' => BooleanValue::getTrue(),
            default => parent::handleSpecificMessage($selector, $args, $definedClasses, $writer, $input)
        };
    }

    /**
     * Check if the selector is a value message matching the block's arity
     *
     * @param string $selector Message selector
     * @return bool True if it's a value message with matching arity
     */
    private function isValueMessage(string $selector): bool
    {
        $arity = $this->blockNode->getArity();

        if ($arity === 0 && $selector === 'value') {
            return true;
        }

        if ($arity > 0) {
            $expectedSelector = str_repeat('value:', $arity);
            return $selector === $expectedSelector;
        }

        return false;
    }

    /**
     * Execute the block with the given arguments
     *
     * @param string $selector Message selector
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return mixed Result of the block execution
     */
    private function executeBlock(
        string $selector,
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): mixed {
        $expectedArity = $this->blockNode->getArity();
        
        // Check arity
        if (count($args) !== $expectedArity) {
            throw new DoesNotUnderstandException(
                "Block expects {$expectedArity} arguments but got " . count($args)
            );
        }

        // Create execution scope inheriting from lexical scope
        $executionScope = new SymbolTable($this->lexicalScope);

        // Bind parameters to arguments
        $parameters = $this->blockNode->getParameters();
        for ($i = 0; $i < $expectedArity; $i++) {
            $paramName = $parameters[$i]['name'];
            if (!is_string($paramName)) {
                throw new \LogicException("Parameter name must be a string");
            }
            $executionScope->define($paramName, $args[$i]);
        }

        // Create visitor and execute the block
        $visitor = new InterpreterVisitor($executionScope, $definedClasses, $writer, $input);
        return $this->blockNode->accept($visitor);
    }

    /**
     * Implement whileTrue: message
     *
     * @param array<mixed> $args Message arguments
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     * @return NilValue Always returns nil
     */
    private function whileTrue(
        array $args,
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): NilValue {
        if (count($args) !== 1) {
            throw new InvalidArgValueException("whileTrue: expects exactly one argument");
        }

        $bodyBlock = $args[0];
        if (!($bodyBlock instanceof BlockValue)) {
            throw new InvalidArgValueException("whileTrue: argument must be a Block");
        }

        // Continue until condition block returns false
        while (true) {
            // Execute this block as condition
            $result = $this->executeBlock('value', [], $definedClasses, $writer, $input);

            // Check if result is a boolean
            if (!($result instanceof TrueValue || $result instanceof FalseValue)) {
                throw new InvalidArgValueException("Block in whileTrue: must return a Boolean");
            }

            // Stop if condition is false
            if ($result instanceof FalseValue) {
                break;
            }

            // Execute body block
            $bodyBlock->executeBlock('value', [], $definedClasses, $writer, $input);
        }

        return NilValue::getInstance();
    }

    /**
     * Create string representation
     *
     * @param DefinedClasses $definedClasses
     * @param StreamWriter $writer
     * @param InputReader $input
     * @return StringValue
     */
    protected function asString(
        DefinedClasses $definedClasses,
        StreamWriter $writer,
        InputReader $input
    ): StringValue {
        $arity = $this->blockNode->getArity();
        $params = implode(',', array_map(fn($p) => $p['name'], $this->blockNode->getParameters()));
        
        return new StringValue(
            "a Block[{$arity}]" . ($params ? "({$params})" : ""),
            $definedClasses,
            $writer,
            $input
        );
    }
}
