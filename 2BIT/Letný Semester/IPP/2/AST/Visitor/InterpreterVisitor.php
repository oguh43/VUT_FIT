<?php

namespace IPP\Student\AST\Visitor;

use IPP\Core\Interface\InputReader;
use IPP\Core\StreamWriter;
use IPP\Student\Exception\DoesNotUnderstandException;
use IPP\Student\Runtime\SymbolTable;
use IPP\Student\Runtime\ClassManagement\DefinedClasses;
use IPP\Student\AST\Node\ProgramNode;
use IPP\Student\AST\Node\ClassNode;
use IPP\Student\AST\Node\MethodNode;
use IPP\Student\AST\Node\BlockNode;
use IPP\Student\AST\Node\AssignNode;
use IPP\Student\AST\Node\ExprNode;
use IPP\Student\AST\Node\LiteralNode;
use IPP\Student\AST\Node\SendNode;
use IPP\Student\AST\Node\VarNode;
use IPP\Student\Runtime\Value\BlockValue;
use IPP\Student\Runtime\Value\ClassValue;
use IPP\Student\Runtime\Value\FalseValue;
use IPP\Student\Runtime\Value\IntegerValue;
use IPP\Student\Runtime\Value\NilValue;
use IPP\Student\Runtime\Value\ObjectValue;
use IPP\Student\Runtime\Value\StringValue;
use IPP\Student\Runtime\Value\TrueValue;
use IPP\Student\Exception\OtherRuntimeException;

/**
 * Visitor that interprets the AST
 */
class InterpreterVisitor implements NodeVisitor
{
    /**
     * Constructor
     *
     * @param SymbolTable $symbolTable Symbol table for variables
     * @param DefinedClasses $definedClasses Available classes
     * @param StreamWriter $writer Output writer
     * @param InputReader $input Input reader
     */
    public function __construct(
        private SymbolTable $symbolTable,
        private DefinedClasses $definedClasses,
        private StreamWriter $writer,
        private InputReader $input
    ) {
    }

    /**
     * Visit a program node
     *
     * @param ProgramNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitProgram(ProgramNode $node): mixed
    {
        // Process each class definition
        foreach ($node->getClasses() as $class) {
            $class->accept($this);
        }

        // Find Main class
        $mainClass = $node->getClass('Main');
        if (!$mainClass) {
            throw new DoesNotUnderstandException("Class Main not found");
        }

        // Find run method
        $runMethod = $mainClass->getMethod('run');
        if (!$runMethod) {
            throw new DoesNotUnderstandException("Method run not found in class Main");
        }

        // Create Main instance
        $mainInstance = new ObjectValue('Main');
        
        // Set self reference and execute the run method
        $this->symbolTable->define('self', $mainInstance);
        return $runMethod->getBlock()->accept($this);
    }

    /**
     * Visit a class node
     *
     * @param ClassNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitClass(ClassNode $node): mixed
    {
        // Classes are registered in the Interpreter class
        return null;
    }

    /**
     * Visit a method node
     *
     * @param MethodNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitMethod(MethodNode $node): mixed
    {
        // Methods are registered in the Interpreter class
        return null;
    }

    /**
     * Visit a block node
     *
     * @param BlockNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitBlock(BlockNode $node): mixed
    {
        // If this is a method block that should be executed, process its statements
        $result = null;
        
        foreach ($node->getAssigns() as $assign) {
            $result = $assign->accept($this);
        }
        
        // Return the result of the last statement, or nil if no statements
        return $result ?? NilValue::getInstance();
    }

    /**
     * Visit an assign node
     *
     * @param AssignNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitAssign(AssignNode $node): mixed
    {
        // Evaluate the expression
        $value = $node->getExpr()->accept($this);
        
        // Store in symbol table
        $this->symbolTable->define($node->getVar()->getName(), $value);
        
        // Return the value
        return $value;
    }

    /**
     * Visit an expression node
     *
     * @param ExprNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitExpr(ExprNode $node): mixed
    {
        $innerNode = $node->getNode();
        
        // If this is a block, we need special handling
        if ($innerNode instanceof BlockNode) {
            // Create a new scope for the block
            $blockScope = new SymbolTable();
            
            // If 'self' exists in the current scope, copy it to the block scope
            if ($this->symbolTable->has('self')) {
                $blockScope->define('self', $this->symbolTable->get('self'));
            }
            
            // Return a block value
            return new BlockValue($innerNode, $blockScope);
        }
        
        // For other node types, just delegate
        return $innerNode->accept($this);
    }

    /**
     * Visit a literal node
     *
     * @param LiteralNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitLiteral(LiteralNode $node): mixed
    {
        // Direct implementation of literal evaluation - no longer calling evaluate()
        return match ($node->getClass()) {
            'String' => new StringValue($node->getValue(), $this->definedClasses, $this->writer, $this->input),
            'Integer' => new IntegerValue((int)$node->getValue(), $this->definedClasses, $this->writer, $this->input),
            'Nil' => NilValue::getInstance(),
            'True' => TrueValue::getInstance(),
            'False' => FalseValue::getInstance(),
            'class' => new ClassValue($node->getValue()),
            default => throw new OtherRuntimeException("Unsupported literal class: {$node->getClass()}")
        };
    }

    /**
     * Visit a send node
     *
     * @param SendNode $node The node to visit
     * @return mixed The result of the message send
     */
    public function visitSend(SendNode $node): mixed
    {
        // Evaluate the receiver
        $receiver = $node->getReceiver()->accept($this);
        
        // If receiver is null, use nil
        if ($receiver === null) {
            $receiver = NilValue::getInstance();
        }
        
        // Evaluate arguments
        $args = [];
        foreach ($node->getArgs() as $arg) {
            $args[] = $arg->accept($this);
        }
        
        // Send the message
        /** @var ObjectValue|StringValue $receiver */
        return $receiver->send(
            $node->getSelector(),
            $args,
            $this->definedClasses,
            $this->writer,
            $this->input
        );
    }

    /**
     * Visit a variable node
     *
     * @param VarNode $node The node to visit
     * @return mixed The variable value
     */
    public function visitVar(VarNode $node): mixed
    {
        return $this->symbolTable->get($node->getName());
    }
}