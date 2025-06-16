<?php

namespace IPP\Student\AST\Visitor;

use IPP\Student\AST\Node\ProgramNode;
use IPP\Student\AST\Node\ClassNode;
use IPP\Student\AST\Node\MethodNode;
use IPP\Student\AST\Node\BlockNode;
use IPP\Student\AST\Node\AssignNode;
use IPP\Student\AST\Node\ExprNode;
use IPP\Student\AST\Node\LiteralNode;
use IPP\Student\AST\Node\SendNode;
use IPP\Student\AST\Node\VarNode;

/**
 * Interface for visitors that process AST nodes
 */
interface NodeVisitor
{
    /**
     * Visit a program node
     *
     * @param ProgramNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitProgram(ProgramNode $node): mixed;

    /**
     * Visit a class node
     *
     * @param ClassNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitClass(ClassNode $node): mixed;

    /**
     * Visit a method node
     *
     * @param MethodNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitMethod(MethodNode $node): mixed;

    /**
     * Visit a block node
     *
     * @param BlockNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitBlock(BlockNode $node): mixed;

    /**
     * Visit an assign node
     *
     * @param AssignNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitAssign(AssignNode $node): mixed;

    /**
     * Visit an expression node
     *
     * @param ExprNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitExpr(ExprNode $node): mixed;

    /**
     * Visit a literal node
     *
     * @param LiteralNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitLiteral(LiteralNode $node): mixed;

    /**
     * Visit a send node
     *
     * @param SendNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitSend(SendNode $node): mixed;

    /**
     * Visit a variable node
     *
     * @param VarNode $node The node to visit
     * @return mixed The result of the visit
     */
    public function visitVar(VarNode $node): mixed;
}
