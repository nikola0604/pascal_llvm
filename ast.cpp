#include "ast.hpp"
#include <iostream>

LLVMContext theContext;
map<string, AllocaInst*> NamedValues;
IRBuilder<> builder(theContext);
Module *theModule;
FunctionPassManager *TheFPM;

int blockCount=0;
ExprAST::~ExprAST()
{}

InnerExprAST::InnerExprAST(ExprAST *a)
{
  _nodes.resize(0);
  _nodes.push_back(a);
}

InnerExprAST::InnerExprAST(ExprAST *a, ExprAST *b)
{
  _nodes.resize(0);
  _nodes.push_back(a); _nodes.push_back(b);
}

InnerExprAST::InnerExprAST(ExprAST *a, ExprAST *b, ExprAST *c)
{
  _nodes.resize(0);
  _nodes.push_back(a); _nodes.push_back(b); _nodes.push_back(c);
}

InnerExprAST::InnerExprAST(ExprAST* a, ExprAST* b, ExprAST* c, ExprAST* d)
{
  _nodes.resize(0);
  _nodes.push_back(a); _nodes.push_back(b); _nodes.push_back(c); _nodes.push_back(d);
}


InnerExprAST::InnerExprAST(vector<ExprAST*> a)
{
  _nodes = a;
}

InnerExprAST::~InnerExprAST()
{
  for(unsigned i=0; i<_nodes.size(); i++)
    delete _nodes[i];
}

FunctionAST::~FunctionAST()
{
  delete Body;
}

Value* NumberExprAST::codegen() const
{
  return ConstantFP::get(theContext, APFloat(Val));
}

Value* VariableExprAST::codegen() const
{
  AllocaInst *tmp = NamedValues[Name];

  if(tmp == NULL)
  {
    cerr << "Promenljiva " + Name + " nije definisana" << endl;
    return NULL;
  }

  return builder.CreateLoad(Type::getDoubleTy(theContext), tmp, Name.c_str());
}

Block::~Block()
{
  for (unsigned i = 0; i < _v.size(); i++)
  {
    delete _v[i];
  }
}

Value* Block::codegen() const
{
  for (unsigned i = 0; i < _v.size(); i++)
  {
    _v[i]->codegen();
  }

  return ConstantInt::get(theContext, APInt(32, 0));
}

Value* AddExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if(l==NULL || r==NULL)
    return NULL;

  return builder.CreateFAdd(l, r, "addtmp");
}

Value* SubExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if(l==NULL || r==NULL)
    return NULL;

  return builder.CreateFSub(l, r, "subtmp");
}

Value* MulExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if(l==NULL || r==NULL)
    return NULL;

  return builder.CreateFMul(l, r, "multmp");
}

Value* DivExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if(l==NULL || r==NULL)
    return NULL;

  return builder.CreateFDiv(l, r, "divtmp");
}

Value* LTExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if (l == NULL || r == NULL)
    return NULL;

  l = builder.CreateFCmpULT(l, r, "lttmp");
  return builder.CreateUIToFP(l, Type::getDoubleTy(theContext) , "booltmp");
}

Value* GTExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if (l == NULL || r == NULL)
    return NULL;

  l = builder.CreateFCmpUGT(l, r, "gttmp");
  return builder.CreateUIToFP(l, Type::getDoubleTy(theContext) , "booltmp");
}

extern Function *PrintfFja;
extern Value *Str;

WriteStatement::~WriteStatement()
{
  delete _r;
}

Value* WriteStatement::codegen() const
{
  Value* r = _r->codegen();

  if (r == NULL)
    return NULL;

  vector<Value*> ArgsV;
  ArgsV.push_back(Str);
  ArgsV.push_back(r);

  return builder.CreateCall(PrintfFja, ArgsV, "printfCall");
}

Value* SeqExprAST::codegen() const
{
  Value *l = _nodes[0]->codegen();
  Value *r = _nodes[1]->codegen();

  if(l == NULL || r == NULL)
    return NULL;

  return r;
}


Value* AssignExprAST::codegen() const
{
  Value *tmp = _nodes[0]->codegen();
  if(tmp == NULL)
    return NULL;

  AllocaInst *Alloca = NamedValues[Name];
  if(Alloca == NULL)
  {
    cerr << "Promenljiva " + Name + " nije definisana" << endl;
    return NULL;
  }

  builder.CreateStore(tmp, Alloca);
  return tmp;
}

// Value* CallExprAST::codegen() const
// {
//   Function *CalleeF = theModule->getFunction(Callee);
//
//   if(!CalleeF)
//   {
//     cerr << "Fja " << Callee << " nije definisana" << endl;
//     return NULL;
//   }
//
//   if(CalleeF->arg_size() != _nodes.size())
//   {
//     cout << "Fja " << Callee << " prima " << CalleeF->arg_size() << " argumenata." << endl;
//     return NULL;
//   }
//
//   vector<Value*> ArgsV;
//   for(unsigned i=0, e=_nodes.size(); i!=e; ++i)
//   {
//     ArgsV.push_back(_nodes[i]->codegen());
//
//     if(!ArgsV.back())
//       return NULL;
//   }
//
//   return builder.CreateCall(CalleeF, ArgsV, "calltmp");
// }

Value* IfExprAST::codegen() const
{
  printf("CondV = codegen()\n\n");
  Value* CondV = _nodes[0]->codegen();
  if(CondV == NULL)
    return NULL;

  printf("CreateFCmpONE\n\n");
  CondV = builder.CreateFCmpONE(CondV, ConstantFP::get(theContext, APFloat(0.0)), "ifcond");
  Function *TheFunction = builder.GetInsertBlock()->getParent();

  printf("Pravim blokove i umece ThenBB u fju\n\n");
  BasicBlock *ThenBB = BasicBlock::Create(theContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(theContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(theContext, "ifcont");

  printf("CreateCondBr\n\n");
  builder.CreateCondBr(CondV, ThenBB, ElseBB);
  builder.SetInsertPoint(ThenBB);
  printf("ThenV = codegen()\n\n");
  Value* ThenV = _nodes[1]->codegen();

  if(ThenV == NULL)
    return NULL;
  printf("CreateBr(MergeBB) ThenBB\n\n");
  builder.CreateBr(MergeBB);
  ThenBB = builder.GetInsertBlock();

  printf("Umecem ElseBB u fju\n\n");
  TheFunction->getBasicBlockList().push_back(ElseBB);
  builder.SetInsertPoint(ElseBB);
  printf("ElseBB = codegen()");
  Value* ElseV = _nodes[2]->codegen();
  if (ElseV == NULL)
    return NULL;

  printf("CreateBr(MergeBB) ElseBB\n\n");
  builder.CreateBr(MergeBB);
  ElseBB = builder.GetInsertBlock();

  TheFunction->getBasicBlockList().push_back(MergeBB);
  builder.SetInsertPoint(MergeBB);

  printf("PHINode\n\n");
  PHINode* PN = builder.CreatePHI(Type::getDoubleTy(theContext), 2, "iftmp");
  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);

  return PN;
}

Value* WhileExprAST::codegen() const
{
  Function* theFunction = builder.GetInsertBlock()->getParent();

  BasicBlock *loopBB = BasicBlock::Create(theContext, "condition", theFunction);

  builder.CreateBr(loopBB);
  builder.SetInsertPoint(loopBB);

  Value* stopV = _nodes[0]->codegen();

  if (!stopV)
    return nullptr;
  stopV = builder.CreateFCmpONE(stopV, ConstantFP::get(theContext, APFloat(0.0)), "loopcond");
  BasicBlock *loopBB1 = BasicBlock::Create(theContext, "loop1", theFunction);
  BasicBlock *afterLoopBB = BasicBlock::Create(theContext, "afterloop", theFunction);
  builder.CreateCondBr(stopV, loopBB1, afterLoopBB);

  builder.SetInsertPoint(loopBB1);

  if (!_nodes[1]->codegen())
  {
    printf("_nodes[1]->codegen DONE\n\n");

    return nullptr;
}

  builder.CreateBr(loopBB);

  builder.SetInsertPoint(afterLoopBB);

  return ConstantFP::get(theContext, APFloat(0.0));
}

// Value* ForExprAST::codegen() const
// {
//   Function *TheFunction = builder.GetInsertBlock()->getParent();
//   // Alociram prostor za promenljivu, na steku, ta instrukcija se unosi u pocetni blok f-je
//   AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
//   // Pamtim staru promenljivu u spoljnjem dosegu
//   AllocaInst *OldAlloca = NamedValues[VarName];
//   NamedValues[VarName] = Alloca;
//
//   // Generise se startni uslov i smesta u promenljivu
//   Value *StartV = _nodes[0]->codegen();
//   if(StartV == NULL)
//     return NULL;
//   builder.CreateStore(StartV, Alloca);
//
//   // Pravim blok petlje i bezuslovnu granu iz trenutnog bloka
//   BasicBlock *LoopBB = BasicBlock::Create(theContext, "loop", TheFunction);
//   builder.CreateBr(LoopBB);
//
//   builder.SetInsertPoint(LoopBB);
//
//   //Generisem telo petlje
//   Value *BodyV = _nodes[3]->codegen();
//   if(BodyV == NULL)
//     return NULL;
//
//   // Generisem inkrementacioni korak petlje, ako nije naveden onda je 1 podrazumevani
//   Value *StepV = NULL;
//   if(_nodes[2] != NULL)
//     StepV = _nodes[2]->codegen();
//   else
//     StepV = ConstantFP::get(theContext, APFloat(1.0));
//   if(StepV == NULL)
//     return NULL;
//
//   // Dohvatam trenutnu vrednost promenljive sa steka, inkrementiram je i vracam nazad na stek
//   Value *curVal = builder.CreateLoad(Type::getDoubleTy(theContext), Alloca, VarName.c_str());
//   Value *nextVar = builder.CreateFAdd(curVal, StepV, "nextvar");
//   builder.CreateStore(nextVar, Alloca);
//
//   Value *CondV = _nodes[1]->codegen();
//   if (CondV == NULL)
//     return NULL;
//   CondV = builder.CreateFCmpONE(CondV, ConstantFP::get(theContext, APFloat(0.0)), "loopcond");
//
//   // Kreiram blok na izlazu iz petlje, i uslovnu granu od bloka petlje(LoopBB) do njega(AfterLoopBB)
//   BasicBlock *AfterLoopBB = BasicBlock::Create(theContext, "afterloop", TheFunction);
//   builder.CreateCondBr(CondV, LoopBB, AfterLoopBB);
//
//   builder.SetInsertPoint(AfterLoopBB);
//
//   if(OldAlloca != NULL)
//     NamedValues[VarName] = OldAlloca;
//   else
//     NamedValues.erase(VarName);
//
//   return ConstantFP::get(theContext, APFloat(0.0));
// }

VarExprAST::~VarExprAST()
{
  for (unsigned i=0;i<V.size(); i++)
    delete V[i].second;
}

Value* VarExprAST::codegen() const
{
  vector<AllocaInst*> oldAllocas;

  Function *TheFunction = builder.GetInsertBlock()->getParent();

  for(unsigned i=0; i<V.size(); i++)
  {
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, V[i].first);
    Value *Tmp = NULL;

    if(V[i].second != NULL)
      Tmp = V[i].second->codegen();
    else
      Tmp = ConstantFP::get(theContext, APFloat(0.0));
    if(Tmp == NULL)
      return NULL;

    oldAllocas.push_back(NamedValues[V[i].first]);
    NamedValues[V[i].first] = Alloca;
    builder.CreateStore(Tmp, Alloca);
  }

  Value *Res = _nodes[0]->codegen();
  if(Res == NULL)
    return NULL;

  for(unsigned i=0; i<oldAllocas.size(); i++)
    if(oldAllocas[i] != NULL)
      NamedValues[V[i].first] = oldAllocas[i];
    else
      NamedValues.erase(V[i].first);

  return Res;
}

Function* PrototypeAST::codegen() const
{
  vector<Type*> Doubles(Args.size(), Type::getDoubleTy(theContext));

  FunctionType *FT = FunctionType::get(Type::getDoubleTy(theContext), Doubles, false);

  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, theModule);

  unsigned Idx = 0;
  for(auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
};

Function* FunctionAST::codegen() const
{
  // Dovlaci se f-ja iz modula, pretrazena po imenu
  Function *TheFunction = theModule->getFunction(Proto.getName());

  // ako vec nije deklarisana, generisi kod deklaracije
  if(!TheFunction)
    TheFunction = Proto.codegen();

  if(!TheFunction)
    return NULL;

  if(!TheFunction->empty())
  {
    cerr << "Nije dozvoljeno predefinisanje f-je " << Proto.getName() << endl;
    return NULL;
  }

  // Kreira se ulazni blok f-je i umece u f-ju
  BasicBlock *BB = BasicBlock::Create(theContext, "entry", TheFunction);
  builder.SetInsertPoint(BB);

  // Argumenti se unose u mapu
  NamedValues.clear();
  for(auto &Arg : TheFunction->args())
  {
    // Kao prva instrukcija f-je se umece alokacija steka za promenlijve
    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());
    // Pokazivac na lokaciju u memoriji se unosi u mapu i mapira na ime promenlijve
    NamedValues[Arg.getName()] = Alloca;
    // Vrednost se unosi u memoriju
    builder.CreateStore(&Arg, Alloca);
  }


  if(Value *RetVal = Body->codegen())
  {
    builder.CreateRet(RetVal);

    verifyFunction(*TheFunction);

    TheFPM->run(*TheFunction);

    return TheFunction;
  }

  TheFunction->eraseFromParent();

  return NULL;
}

void InitializeModuleAndPassManager(void)
{
  theModule = new Module("my_module", theContext);

  TheFPM = new FunctionPassManager(theModule);

  TheFPM->add(createInstructionCombiningPass());
  TheFPM->add(createReassociatePass());
  TheFPM->add(createGVNPass());
  TheFPM->add(createCFGSimplificationPass());
  TheFPM->add(createPromoteMemoryToRegisterPass());

  TheFPM->doInitialization();
}

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, const string &VarName)
{
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getDoubleTy(theContext), 0, VarName.c_str());
}
