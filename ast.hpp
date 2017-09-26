#ifndef __AST_HPP__
#define __AST_HPP__ 1

#include <string>
#include <vector>

using namespace std;

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;
using namespace llvm::legacy;

class ExprAST
{
public:
  virtual ~ExprAST();
  virtual Value* codegen() const = 0;
};

class NumberExprAST : public ExprAST
{
public:
  NumberExprAST(int v):Val(v)
  {}
  Value* codegen() const;
private:
  double Val;
};

class VariableExprAST : public ExprAST
{
public:
  VariableExprAST(string s):Name(s)
  {}
  Value* codegen() const;
private:
  string Name;
};

class InnerExprAST : public ExprAST
{
public:
  InnerExprAST(ExprAST *a);
  InnerExprAST(ExprAST *a, ExprAST *b);
  InnerExprAST(ExprAST *a, ExprAST *b, ExprAST *c);
  InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c, ExprAST* d);
  InnerExprAST(vector<ExprAST*> a);
  ~InnerExprAST();
protected:
  vector<ExprAST*> _nodes;
private:
  InnerExprAST(const InnerExprAST &i);
  InnerExprAST& operator=(const InnerExprAST &i);
};

class Block : public ExprAST
{
public:
  Block(vector<ExprAST*> v):_v(v)
  {}
  ~Block();
  Value* codegen() const;
private:
  Block(const Block&);
  Block& operator=(const Block&);
  vector<ExprAST*> _v;
};

class AddExprAST : public InnerExprAST
{
public:
  AddExprAST(ExprAST *a, ExprAST *b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class SubExprAST : public InnerExprAST
{
public:
  SubExprAST(ExprAST* a, ExprAST* b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class MulExprAST : public InnerExprAST
{
public:
  MulExprAST(ExprAST* a, ExprAST* b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class DivExprAST : public InnerExprAST
{
public:
  DivExprAST(ExprAST* a, ExprAST* b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class LTExprAST : public InnerExprAST
{
public:
  LTExprAST(ExprAST *a, ExprAST *b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class GTExprAST : public InnerExprAST
{
public:
  GTExprAST(ExprAST *a, ExprAST *b):InnerExprAST(a, b)
  {}
  Value* codegen() const;
};

class WriteStatement : public ExprAST
{
public:
  WriteStatement(ExprAST *r):_r(r)
  {}
  ~WriteStatement();
  Value* codegen() const;
private:
  WriteStatement(const WriteStatement&);
  WriteStatement& operator=(const WriteStatement&);
  ExprAST *_r;
};

class AssignExprAST : public InnerExprAST
{
public:
  AssignExprAST(string s, ExprAST *a):InnerExprAST(a), Name(s)
  {}
  Value* codegen() const;
private:
  string Name;
};

class IfExprAST : public InnerExprAST
{
public:
  IfExprAST(ExprAST *a, ExprAST *b, ExprAST *c):InnerExprAST(a, b, c)
  {}
  Value* codegen() const;
};

class WhileExprAST : public InnerExprAST
{
public:
  WhileExprAST(ExprAST *a, ExprAST *b):InnerExprAST(a,b)
  {}
  Value* codegen() const;
private:
  WhileExprAST(const WhileExprAST&);
  WhileExprAST& operator=(const WhileExprAST&);
};

class VarExprAST : public InnerExprAST
{
public:
  VarExprAST(vector< pair<string, ExprAST*> > v, ExprAST *a):InnerExprAST(a), V(v)
  {}
  Value* codegen() const;
  Value* codegen(map<string, AllocaInst*>*) const;
  ~VarExprAST();
private:
  VarExprAST& operator=(const VarExprAST&);
  VarExprAST(const VarExprAST&);
  vector< pair<string, ExprAST*> > V;
};

class PrototypeAST
{
public:
  PrototypeAST(string n, vector<string> a):Name(n), Args(a)
  {}
  Function* codegen() const;
  string getName() const {return Name;}
private:
  string Name;
  vector<string> Args;
};

class FunctionAST
{
public:
  FunctionAST(PrototypeAST p, ExprAST *b):Proto(p), Body(b)
  {}
  ~FunctionAST();
  Function* codegen() const;
private:
  FunctionAST(const FunctionAST &f);
  FunctionAST& operator=(const FunctionAST &f);
  PrototypeAST Proto;
  ExprAST *Body;
};

void InitializeModuleAndPassManager(void);
AllocaInst* CreateEntryBlockAlloca(Function *TheFunction, const string &VarName);

#endif
