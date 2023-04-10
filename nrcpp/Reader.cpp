// ���������� ��������������� ������� ������� - Reader.cpp


#pragma warning(disable: 4786)
#include <nrc.h>
using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "Object.h"
#include "Scope.h"
#include "LexicalAnalyzer.h"
#include "Class.h"
#include "Manager.h"
#include "Parser.h"
#include "Body.h"
#include "Reader.h"
#include "ExpressionMaker.h"
#include "Maker.h"
#include "MemberMaker.h"
#include "Coordinator.h"


// ������� �����
int ExpressionReader::StackOverflowCatcher::stackDeep = 0;


// �������� ��������� �������, ��������� �� � ����������
const Lexem &DeclaratorReader::NextLexem()
{
	// ���� ��������� ������ �� ����, ������� ������� �� ����
	if( canUndo )
	{
		if( !undoContainer.empty() )
		{
			lastLxm = undoContainer.front();
			lexemContainer.push_back(lastLxm);
			undoContainer.pop_front();						
			return lastLxm;
		}

		else
			canUndo = false;
	}

	lexemContainer.push_back(lexicalAnalyzer.NextLexem());
	return lastLxm = lexemContainer.back();
}


// ������� ���������� ������� � �����
void DeclaratorReader::BackLexem()
{
	// ���� ��������� ������ �� ����, ������� ������� � ����
	if( canUndo )	
	{
		lexemContainer.pop_back();
		undoContainer.push_front(lastLxm);
	}

	else
	{
		lexicalAnalyzer.BackLexem();
		lexemContainer.pop_back();
	}
}


// �������� ��������� �������, ��������� �� � ����������
const Lexem &QualifiedConstructionReader::NextLexem()
{
	// ���� ��������� ������ �� ����, ������� ������� �� ����
	if( canUndo )
	{
		if( !undoContainer->empty() )
		{
			lastLxm = undoContainer->front();
			lexemContainer.push_back(lastLxm);
			undoContainer->pop_front();						
			return lastLxm;
		}

		else
			canUndo = false;
	}

	lexemContainer.push_back(lexicalAnalyzer.NextLexem());
	return lastLxm = lexemContainer.back();
}


// ������� ���������� ������� � �����
void QualifiedConstructionReader::BackLexem()
{
	// ���� ��������� ������ �� ����, ������� ������� � ����
	if( canUndo )	
	{
		lexemContainer.pop_back();
		undoContainer->push_front(lastLxm);
	}

	else
	{
		lexicalAnalyzer.BackLexem();
		lexemContainer.pop_back();
	}
}


// ������� ������������� ����
void DeclaratorReader::ReadTypeSpecifierList()
{
	bool hasBaseType = false;
	Lexem lxm;

	for( ;; )
	{
		lxm = NextLexem();		

		// ����� �� ��������������: ������� ���, cv-������������, 
		// ������������ ��������, ����������� �������, ����������� �����,
		// ������������� ������� (inline, virtual, explicit), 
		// ������������ friend
		if( lxm == KWAUTO	  || lxm == KWBOOL		|| lxm == KWCHAR		||
			lxm == KWCONST	  || lxm == KWDOUBLE	|| lxm == KWEXPLICIT	||
			lxm == KWEXTERN   || lxm == KWFLOAT		|| lxm == KWFRIEND		||
			lxm == KWINLINE   || lxm == KWINT		|| lxm == KWLONG		||
			lxm == KWMUTABLE  || lxm == KWREGISTER 	|| lxm == KWSHORT		||
			lxm == KWSIGNED   || lxm == KWSTATIC	|| lxm == KWTYPEDEF		||
			lxm == KWUNSIGNED || lxm == KWVIRTUAL	||
			lxm == KWVOID	  || lxm == KWVOLATILE	|| lxm == KWWCHAR_T	 )		
		{
			tslPkg->AddChildPackage( new LexemPackage(lxm) );
			if(lxm == KWBOOL || lxm == KWCHAR || lxm == KWDOUBLE ||
				lxm == KWFLOAT || lxm == KWINT || lxm == KWVOID || lxm == KWWCHAR_T ||
				lxm == KWSHORT || lxm == KWLONG || lxm == KWUNSIGNED || lxm == KWSIGNED ) 
				hasBaseType = true;
		}
	 
		// ������������� ������, ����� ��������� ��� ����� ����
		else if( lxm == KWCLASS || lxm == KWSTRUCT ||
				 lxm == KWENUM  || lxm == KWUNION )
		{
			if( hasBaseType )
				break;

			tslPkg->AddChildPackage( new LexemPackage(lxm) );

			lxm = NextLexem();
			BackLexem();
			if( lxm == '{' || lxm == ':' )		// ����� ���������� �����
				return;
			
			QualifiedConstructionReader qcr(lexicalAnalyzer, false, true);
			PNodePackage np = qcr.ReadQualifiedConstruction();	// ��������� ��� ������

			if( qcr.IsPointerToMember() )
				throw qcr.GetLexemContainer().back();
			tslPkg->AddChildPackage( np.Release() );
			hasBaseType = true;
		}	

		// ����� ���� ���, ������������� ����� ���� �����
		else if( lxm == NAME || lxm == COLON_COLON )
		{
			// ���� ������� ��� ��� ������ - ������ ����� ���� � ���������������,
			// � ������� ������� ������� � �����
			if( hasBaseType )			
				break;			

			QualifiedConstructionReader qcr(lexicalAnalyzer,
				false, false, PLexemContainer(NULL), canBeExpression);
			if( canUndo )
			{
				BackLexem();
				qcr.LoadToUndoContainer(undoContainer);
				undoContainer.clear();
				canUndo = false;
			}

			else
				BackLexem();
			PNodePackage np = qcr.ReadQualifiedConstruction();

			// ���� ���� ������� ���������, ��������� ��������� �������
			// � ����������, � ���������� �������������� ������
			if( qcr.IsExpression() )
			{
				INTERNAL_IF( !canBeExpression );				
				LoadToLexemContainer(qcr.GetLexemContainer());
				throw qcr.GetLexemContainer().back();
			}

			// ��� ������ �� ���, ��������� ��� ��������� ������� 
			// � ���������� ������, ��� ���������� ��������������
			// ������� ReadDeclarator
			if( qcr.IsPointerToMember() || !qcr.IsTypeName() )
			{
				LoadToUndoContainer(qcr.GetLexemContainer());						
				return;
			}

			// �������� ��������� ������� � ����������			
			LoadToLexemContainer(qcr.GetLexemContainer());
			LoadToUndoContainer(qcr.GetUndoContainer());
			np->SetPackageID(PC_QUALIFIED_TYPENAME);
			tslPkg->AddChildPackage( np.Release() );
			hasBaseType = true;
		}

		// ����� ���� ��������� �������, � ���� ������ ����� ���������,
		// ���� �� ���������� ������� extern, � ���� �� ��������� �����. ��������
		else if( lxm == STRING && lexicalAnalyzer.PrevLexem() == KWEXTERN )
		{
			if( !(::GetCurrentSymbolTable().IsGlobalSymbolTable() ||
				  ::GetCurrentSymbolTable().IsNamespaceSymbolTable()) )
			{
				theApp.Error(lxm.GetPos(), 
					"������������ ���������� ����� ���������� ������ ���������");
				continue;
			}

			tslPkg->AddChildPackage( new LexemPackage(lxm) );
			if( NextLexem() == '{' && tslPkg->GetChildPackageCount() == 2 )
			{
				const_cast<Parser&>(theApp.GetTranslationUnit().GetParser()).
					SetLinkSpecification(Parser::LS_C);
				return;
			}

			else
				BackLexem();
		}

		else
			break;
	}

	// ������� ��������� ��������� �������
	BackLexem();	
}


// ������� ��������� ����������, ������������ � ������, ����� �
// ���������� ����� ���� ��������� ������������. ����� ����������
// �������������� �������� ���� ���� �������������� ������
PNodePackage DeclaratorReader::ReadNextDeclarator()
{
	declPkg = new NodePackage(PC_DECLARATOR);
	crampLevel = 0;
	nameRead = false;
	initializatorList = NULL;		// ������� ������ ���������������
	ReadDeclarator();
	return declPkg;
}


// ������� ����������, �������� ������� ������� ��������� ��� ������
// �� ������, ReadNextDeclarator - �������� ��. ����� �������
// ������ declPkg - ������ ����������� ������
void DeclaratorReader::ReadDeclarator( )
{
	OverflowController oc(MAX_PARSER_STACK_DEEP);		// �������� ������������	

	Lexem lxm = NextLexem();
	if( lxm == '*' )
	{	
		PNodePackage p = new NodePackage(0);		
		ReadCVQualifierSequence(p);

		ReadDeclarator();
		declPkg->AddChildPackage(new LexemPackage(lxm));

		if( !p->IsNoChildPackages()  )
		{
			Lexem cv1 = ((LexemPackage *)p->GetChildPackage(0))->GetLexem();

			if( p->GetChildPackageCount() > 1 )
			{
				Lexem cv2 = ((LexemPackage *)p->GetChildPackage(1))->GetLexem();
				declPkg->AddChildPackage( new LexemPackage(cv1) );
				declPkg->AddChildPackage( new LexemPackage(cv2) );
			}

			else
				declPkg->AddChildPackage( new LexemPackage(cv1) );									
		}
	}

	else if( lxm == '&' )
	{
		ReadDeclarator();
		declPkg->AddChildPackage(  new LexemPackage(lxm) );
	}

	// � ���� ����� ����������� ���� ���������� ��������. ��� ����������
	// ������������ ����, ���������� ��������������� ����� '��������� � �������'
	// � '���������� �������'. �������� ���������, ������������ �������� ���������
	// �������, �� ������ � ������ ���� � ������� ������ ���������� ��� ������ ������.
	else if( lxm == '(' )			
	{				
		if( declType == DV_CAST_OPERATOR_DECLARATION )
		{
			BackLexem();
			return;
		}

		// ��������� ������ ���� ������, ����� �� ����� � ��� ������� ���������
		// ���������������
		INTERNAL_IF( !undoContainer.empty() );		
		
		DeclaratorReader dr(DV_PARAMETR, lexicalAnalyzer);
		dr.ReadTypeSpecifierList();
		PNodePackage np = dr.GetTypeSpecPackage(); 
				
		// ��� ������ ���, ������ ��������� ��� � ��������� ������
		// � ��������� �������� �������
		if( !np->IsNoChildPackages() || dr.lastLxm == ELLIPSES )
		{
			// ��������� ��������� �������, ����� ������������� ������
			dr.NextLexem();
			// ���������� ��� ����������
			dr.lexemContainer.insert(dr.lexemContainer.end(), dr.undoContainer.begin(),
				dr.undoContainer.end());
			// �� undo-����������, ������ ����� ����������� ��������
			LoadToUndoContainer(dr.GetLexemContainer());	
			lastLxm = lxm;
		}

		// ����� ���� ��� ������ ')', ������ ����� ���� � ������ ������ ����������
		else if( dr.lastLxm == ')' )
		{
			lastLxm = lxm;
		}

		// ���� ������ �� �������, ��������� ����������
		else
		{		
			// � ���� ����� , dr ��� ������� ������ ��� ���������, ����
			// ��������� �� ����, ��� ��� ������� �������� � undo-����������		
			LoadToUndoContainer(dr.undoContainer);	
			lastLxm = lxm;

			crampLevel++;
			ReadDeclarator();
			crampLevel--;
			if( lastLxm != ')' )
				throw lastLxm;
			NextLexem();
		}
	}


	// ��� ����� ����: ��������� �� ����, ��� �������������,
	// ������� �� � ���������	
	else if( lxm == NAME || lxm == COLON_COLON )
	{
		bool nq = false, ns = false;
		if( declType == DV_PARAMETR || declType == DV_CAST_OPERATION )
			nq = ns = true;

		// ������ � ��������� ����������. ��� using-���������� ������ ������
		// ��������� ����������������� ���
		else if( declType == DV_LOCAL_DECLARATION )
			nq = true;
			
		BackLexem();
		QualifiedConstructionReader qcr(lexicalAnalyzer, nq, ns);
		qcr.LoadToUndoContainer( undoContainer );	// �������� ��� � ���������
		undoContainer.clear();
		PNodePackage np = qcr.ReadQualifiedConstruction();
		LoadToLexemContainer(qcr.GetLexemContainer());


		// ���� ��������� �� ����
		if( qcr.IsPointerToMember() )
		{			
			ReadCVQualifierSequence(np);		// ��������� cv-�������������
			ReadDeclarator();
		
			declPkg->AddChildPackage( np.Release() );		// ��������� ��������� � ������ �����. �����			
		}

		// ����� ����� ���� � ���������������, ��������� ��� � �������
		else 
		{
			// ���������, ����� �� ������������ ����������� �������
			// � �������� ���������
			if( declType != DV_GLOBAL_DECLARATION && declType != DV_LOCAL_DECLARATION && 
				declType != DV_CLASS_MEMBER && declType != DV_PARAMETR ) 
				throw lxm;

			declPkg->AddChildPackage( np.Release() );

			// ��������� ��������� �������, �.�. 
			// ReadQualifiedConstruction �� ������� � ������
			NextLexem();	
			
			// �������������, ��� ��� �������
			nameRead = true;
		}
	}	

	// ���� ����� ���� � ���������� �����������, ���������� ����� ��� ���������
	// ����������, ����� ��������� ����������� ������� �����
	else if( lxm == KWOPERATOR || lxm == '~' ) 
	{
		// ���������, ����� �� ������������ ����������� �������
		// � �������� ���������
		if( declType != DV_GLOBAL_DECLARATION &&
			declType != DV_LOCAL_DECLARATION && 
			declType != DV_CLASS_MEMBER ) 
			throw lxm;

		BackLexem();
		QualifiedConstructionReader qcr(lexicalAnalyzer);
		PNodePackage np = qcr.ReadQualifiedConstruction();

		LoadToLexemContainer(qcr.GetLexemContainer());
		declPkg->AddChildPackage( np.Release() );	// ��������� ��������� � ������ �����. �����			
		lastLxm = NextLexem();
		nameRead = true;
	}	

	// � ����� ������ �������������� ��������� ����� ����������� [], ()
	ReadDeclaratorTailPart();	
}


// ��������� ��������� ����� �����������, ���� �� ���������� ����
void DeclaratorReader::ReadDeclaratorTailPart()
{
	Lexem lxm = lastLxm;

	// ��������� ����������� ����������� ����
	bool in;
	for( in = false; ;in++ )
	{	
		// ��������� ������� �������� �������
		if( lxm == '(' )
		{  			
			if( declType == DV_CAST_OPERATOR_DECLARATION )
				break;
			
			PNodePackage np = ReadFunctionPrototype();
			if( !np.IsNull() )
			{
				INTERNAL_IF( np->GetPackageID() != PC_FUNCTION_PROTOTYPE || 
							 np->IsNoChildPackages() );		
				declPkg->AddChildPackage(np.Release());
			}

			// ����� ��� ������ ������ ���������������, � �� �������
			else
			{
				INTERNAL_IF( initializatorList.IsNull() || initializatorList->empty() );
				break;
			}
		}

		else if( lxm == '[' )
		{
			Lexem tmp = NextLexem();
			PNodePackage ar = new NodePackage(PC_ARRAY) ;

			// ��������� ���������
			if( tmp != ']' )
			{
				BackLexem();
				ExpressionReader expReader( lexicalAnalyzer, 
					undoContainer.empty() ? NULL : &undoContainer, true);
				expReader.Read();				
				INTERNAL_IF( expReader.GetResultOperand().IsNull() );

				// ������� �����-���������
				ar->AddChildPackage( new LexemPackage(lxm) );
				ar->AddChildPackage( new ExpressionPackage( expReader.GetResultOperand()) );
				tmp = lexicalAnalyzer.LastLexem();
				if( tmp != ']' )
					throw tmp;
				ar->AddChildPackage( new LexemPackage(tmp) );
				declPkg->AddChildPackage(ar.Release());
			}

			// ����� ������ ������, ��������� ���
			else
			{
				ar->AddChildPackage( new LexemPackage(lxm) );
				ar->AddChildPackage( new LexemPackage(tmp) );
				declPkg->AddChildPackage(ar.Release());
			}
		}

		else
			break;		

		lxm = NextLexem();
	}

	// ���� in ���������� � 
	if( declType == DV_CAST_OPERATOR_DECLARATION && in )
		BackLexem();
		
}

// ������� �������� �������
PNodePackage DeclaratorReader::ReadFunctionPrototype()
{
	PNodePackage np = new NodePackage(PC_FUNCTION_PROTOTYPE);			
	np->AddChildPackage( new LexemPackage( lastLxm ) );	// �������� '('
	Lexem lxm;
	
	// �������� ��������� ������� ��� ��������
	lxm = NextLexem();

	// ���� ������� ������ �������, ����� ��������� ��� ���������� ����������,
	// �������� ������� ������ �������������
	if( crampLevel == 0 &&
		(declType == DV_GLOBAL_DECLARATION || declType == DV_LOCAL_DECLARATION) &&
		(lxm != ')' && lxm != ELLIPSES) &&
		undoContainer.empty() )
	{
		BackLexem();
		TypeExpressionReader ter(lexicalAnalyzer, true);
		ter.Read(true);
		INTERNAL_IF( ter.GetResultPackage() == NULL );

		// ������ ���������, ���� ����� ���������, ��������� ��� ��������������,
		if( ter.GetResultPackage()->IsExpressionPackage() )
		{
			initializatorList = new ExpressionList;
			initializatorList->push_back( 
				static_cast<const ExpressionPackage &>(*ter.GetResultPackage()).GetExpression());

			// ��������� ���� ������
			for( ;; )
			{
				if( lexicalAnalyzer.LastLexem() == ')' )
					break;
				if( lexicalAnalyzer.LastLexem() != ',' )
					throw lexicalAnalyzer.LastLexem();

				// ��������� ��������� ���������
				ExpressionReader er(lexicalAnalyzer, NULL, true);
				er.Read();

				// �������� ���������
				initializatorList->push_back( er.GetResultOperand() );
			}

			// ��������� ';' ��� ������� � �������
			lexicalAnalyzer.NextLexem();
			return NULL;
		}

		// ����� ����� ����������, �.�. ��������. ������� �����
		// � ���������� � ���������� ���������� � �����
		else
		{
			INTERNAL_IF( !ter.GetResultPackage()->IsNodePackage() );

			// ����� �������� �������� �� ���������
			PNodePackage prm = new NodePackage(PC_PARAMETR);
			const NodePackage *dcl = static_cast<const NodePackage*>(ter.GetResultPackage());
			INTERNAL_IF( dcl->GetChildPackageCount() != 2 );

			prm->AddChildPackage( const_cast<Package*>(dcl->GetChildPackage(0)) );
			prm->AddChildPackage( const_cast<Package*>(dcl->GetChildPackage(1)) );
	
			// ���������, ���� ��������� ��������� ������� '=', ������ �������
			// ����� ������� �������� ��������� �� ���������
			if( lexicalAnalyzer.LastLexem() == '=' )
			{
				// ��������� ��������� ���������
				ExpressionReader er(lexicalAnalyzer, NULL, true);
				er.Read();
			
				// ��������� ����� � ���������� � ���������
				prm->AddChildPackage( new ExpressionPackage(er.GetResultOperand()) );
			}

			// ��������� �������� � ������ ������
			np->AddChildPackage( prm.Release() );
			lxm = lexicalAnalyzer.LastLexem() == ',' ? 
				lexicalAnalyzer.NextLexem() : lexicalAnalyzer.LastLexem();
		}
	}

	// ���� ������ ����������
	if( lxm != ')' )
	for( ;; )
	{			
		if( lxm == ELLIPSES )	// ... 
		{
			Lexem tmp = NextLexem();
			if( tmp != ')' )
				throw tmp;
			else
			{
				np->AddChildPackage( new LexemPackage(lxm) );
				break;
			}
		}

		else
			BackLexem();

		DeclaratorReader dr(DV_PARAMETR, lexicalAnalyzer);
		dr.LoadToUndoContainer( undoContainer );
		undoContainer.clear();
		canUndo = false;		

		dr.ReadTypeSpecifierList();						// ��������� ������ �������������� ����
		PNodePackage tsl = dr.GetTypeSpecPackage();
		if( tsl->IsNoChildPackages() )
			throw lexicalAnalyzer.LastLexem();

		PNodePackage dcl = dr.ReadNextDeclarator(),			// ��������� ����������
					 prm = new NodePackage(PC_PARAMETR);
		
		prm->AddChildPackage(tsl.Release());
		prm->AddChildPackage(dcl.Release());
		
		LoadToLexemContainer(dr.GetLexemContainer());

		// ������� �������� �� ��������� ��� ���������
		if( lexicalAnalyzer.LastLexem() == '=' )
		{
			// ��������� �� ��������� ��������� ������ � ���������,
			// ���������� � ����������� ������
			if( declType != DV_GLOBAL_DECLARATION &&
				declType != DV_LOCAL_DECLARATION  &&
				declType != DV_CLASS_MEMBER )
				throw lexicalAnalyzer.LastLexem();

			// ��������� �������� ��������� �� ���������
			ExpressionReader er(lexicalAnalyzer, NULL, true);
			er.Read();
			
			// ��������� ����� � ���������� � ���������
			prm->AddChildPackage( new ExpressionPackage(er.GetResultOperand()) );
		}
		
		np->AddChildPackage( prm.Release() );			// ������� ��������� �������� � ���������	
		if( lexicalAnalyzer.LastLexem() != ',' && lexicalAnalyzer.LastLexem() != ')' )
			throw lexicalAnalyzer.LastLexem();

		if( lexicalAnalyzer.LastLexem() == ')' )
			break;

		// ��������� ���������
		lxm = NextLexem();
	}

	np->AddChildPackage( new LexemPackage(lexicalAnalyzer.LastLexem()) );

	// ������ ����� ������������� ������� ������ �������������� 
	Lexem c, v; 
	ReadCVQualifierSequence(np);	

	// ����� ����� ���� ������ ������������ �������������� ��������
	lxm = NextLexem();

	// ��������� ������ �������������� ��������
	if( lxm == KWTHROW )
	{
		PNodePackage throwSpec = new NodePackage(PC_THROW_TYPE_LIST);
		
		if( (lxm = NextLexem()) != '(' )
			throw lxm;

		throwSpec->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();

		// ������ throw-������������
		if( lxm == ')' )		
			throwSpec->AddChildPackage( new LexemPackage(lxm) );

		// ����� ��������� ������ �����
		else
		{
			BackLexem();
			while(true)
			{
				throwSpec->AddChildPackage(ReadThrowSpecType().Release());
				lxm = lexicalAnalyzer.LastLexem();

				if( lxm == ')' )
				{
					throwSpec->AddChildPackage( new LexemPackage(lxm) );
					break;
				}

				else if( lxm != ',' )
					throw lxm;
			}
		}

		np->AddChildPackage(throwSpec.Release());
	}

	else
		BackLexem();
	return np;
}


// ������� ��� � throw-������������ � ������� ���
PNodePackage DeclaratorReader::ReadThrowSpecType()
{
	DeclaratorReader dr(DV_CAST_OPERATION, lexicalAnalyzer);
	
	dr.ReadTypeSpecifierList();						// ��������� ������ �������������� ����
	PNodePackage tsl = dr.GetTypeSpecPackage();
	if( tsl->IsNoChildPackages() )
		throw lexicalAnalyzer.LastLexem();

	PNodePackage dcl = dr.ReadNextDeclarator(),			// ��������� ����������
				 tht = new NodePackage(PC_THROW_TYPE);
		
	tht->AddChildPackage(tsl.Release());
	tht->AddChildPackage(dcl.Release());
		
	LoadToLexemContainer(dr.GetLexemContainer());
	return tht;
}


// ������� ������������������ �� cv-��������������, � ��������� �� 
// � ������
void DeclaratorReader::ReadCVQualifierSequence( PNodePackage &np )
{
	Lexem cv;
	bool c = false, v = false;
	for( ;; )
	{
		cv = NextLexem();
		if( cv == KWCONST && c == false ) 
			np->AddChildPackage( new LexemPackage(cv) ), c = true;

		else if( cv == KWVOLATILE && v == false ) 
			np->AddChildPackage( new LexemPackage(cv) ), v = true;

		else 
			break;
	}

	BackLexem();
}


// ���� ������ ��� - �����, ������������, typedef, ������������������ ������
bool QualifiedConstructionReader::IsTypeName( ) const
{
	return QualifiedNameManager(&*resultPackage, NULL).IsTypeName(); 
}


// ������� ����������������� �����������
PNodePackage QualifiedConstructionReader::ReadQualifiedConstruction()
{
	bool wasQual = false;
	Lexem lxm = NextLexem();

	// ���� ���� ������� '::', ������� �� � ������� ���
	if( lxm == COLON_COLON )		
	{
		resultPackage->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();		

		// ���� ��������� ��������� � ��������� ������� - new ��� delete,
		// �������������� ������ �� ���������
		if( noErrorOnExp && (lxm == KWNEW || lxm == KWDELETE) )
			return readExpression = true, NULL;

		// ���� ��� ����� ����� '::' - ��� �������������� ������	
		if( lxm != NAME && 
			(!noQualified && !noSpecial && lxm != KWOPERATOR) )
			throw lxm;
		wasQual = true;
	}


	// ����� ������ ���� ���, ���� �����, ���� ����� '::'
	if( lxm == NAME )
		resultPackage->AddChildPackage( new LexemPackage(lxm) );	

	// ������� ������� ��������, � ����� ��� ��������, ����� 
	else if( lxm == KWOPERATOR )
	{
read_operator:

		// ���� �� �� ����� ��������� ���������
		if( noSpecial || (noQualified && wasQual) )
			throw lxm;

		// �������� ������� ��������, � ������ �������������� ������,
		// ���� ����� ���������
		NodePackage *op = ReadOverloadOperator().Release();
		resultPackage->AddChildPackage( op );
		return resultPackage;		// �������, ��� �������
	}

	// �������� ����������
	else if( lxm == '~' )
	{
read_destructor:

		// �� ����� ��������� �����������
		if( noSpecial || (noQualified && wasQual) )
			throw lxm;

		PNodePackage dtor = new NodePackage( PC_DESTRUCTOR );
		dtor->AddChildPackage(new LexemPackage(lxm));
		
		lxm = NextLexem();
		if( lxm != NAME )
			throw lxm;
		
		dtor->AddChildPackage(new LexemPackage(lxm));
		resultPackage->AddChildPackage(dtor.Release());
		return resultPackage;		// �������, ���������� ������
	}

	// ����� �������������� ������
	else		
		throw lxm;	

	// ���� ���������� ��������� �����, ��������� ��� �������,
	// ������ � ������ ���� ���������� �������� ����� template 
	// ����� '::'
	for( ;; )
	{
		lxm = NextLexem();
		if( lxm != COLON_COLON )
		{
			if( wasQual && noQualified )
				throw lxm;
			break;
		}
		else
			wasQual = true;

		resultPackage->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();

		// ��������� ��� �������
		if( lxm == KWTEMPLATE )
		{
		}

		// ����� ������ �������� ���
		else
		{
			// ����� ���, ��������� ��� � �����
			if( lxm == NAME   )
				resultPackage->AddChildPackage( new LexemPackage(lxm) );

			// ���� ��������� ������� ��������� �� ����
			else if( lxm == '*' )
			{
				resultPackage->SetPackageID( PC_POINTER_TO_MEMBER );
				resultPackage->AddChildPackage( new LexemPackage(lxm) );
				return resultPackage;				
			}

			else if( lxm == KWOPERATOR && !noSpecial )
				goto read_operator;

			else if( lxm == '~' && !noSpecial )
				goto read_destructor;

			else
				throw lxm;				
		}
	}

	BackLexem();		// ���������� ������. ��������� ������� � �����	
	return resultPackage;
}


// ������� ������������� ��������
PNodePackage QualifiedConstructionReader::ReadOverloadOperator( )
{
	PNodePackage op( new NodePackage( PC_OVERLOAD_OPERATOR ) );
	
	// ���������� 'operator'	
	op->AddChildPackage( new LexemPackage(
		lastLxm == KWOPERATOR ? lastLxm : lexicalAnalyzer.LastLexem()) );
	Lexem lxm = NextLexem();

	// ���� �������� ������� ���������� []
	if( lxm == KWNEW || lxm == KWDELETE )
	{
		op->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();
		if( lxm != '[' )
		{
			BackLexem();
			return op;
		}
				
		op->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();
		if( lxm != ']' )
			throw lxm;
		op->AddChildPackage( new LexemPackage(lxm) );

		return op;
	}

	// ���� �������� '()' ��� '[]'
	else if( lxm == '(' || lxm == '[' )
	{
		int nop = lxm == '(' ? ')' : ']';
		op->AddChildPackage( new LexemPackage(lxm) );
		lxm = NextLexem();
		if( lxm != nop )
			throw lxm;
				
		op->AddChildPackage( new LexemPackage(lxm) );
		return op;
	}


	// ����� �������� ������� �� ����� �������
	else if( lxm == '+' || lxm == '-'  || lxm == '*'				||
			 lxm == '/' || lxm == '%'  || lxm == '^'				||
			 lxm == '&' || lxm == '|'  || lxm == '~'				||
			 lxm == '!' || lxm == '='  || lxm == '<'				||
			 lxm == '>'												|| 
			 lxm == PLUS_ASSIGN		   || lxm == MINUS_ASSIGN		||
			 lxm == MUL_ASSIGN		   || lxm == DIV_ASSIGN			|| 
			 lxm == PERCENT_ASSIGN     || lxm == LEFT_SHIFT_ASSIGN  ||
			 lxm == RIGHT_SHIFT_ASSIGN || lxm == AND_ASSIGN			|| 
			 lxm == XOR_ASSIGN		   || lxm == OR_ASSIGN			|| 
			 lxm == LEFT_SHIFT		   || lxm == RIGHT_SHIFT		|| 
			 lxm == LESS_EQU		   || lxm == GREATER_EQU		|| 
			 lxm == EQUAL			   || lxm == NOT_EQUAL			||
			 lxm == LOGIC_AND		   || lxm == LOGIC_OR			|| 
			 lxm == INCREMENT		   || lxm == DECREMENT		    || 
			 lxm == ARROW			   || lxm == ARROW_POINT		||
			 lxm == ',' )
	{
		op->AddChildPackage( new LexemPackage(lxm) );
		return op;
	}

	// ����� �������� ������� ���, ����� �������� ���������� ����
	else
	{
		DeclaratorReader dr(DV_CAST_OPERATOR_DECLARATION, lexicalAnalyzer);

		// ��������� ����������, ��� ����� ��� �������� ��������� ���������� ����
		PNodePackage ct = new NodePackage(PC_CAST_TYPE);		
		
		BackLexem();				
		dr.LoadToUndoContainer( *undoContainer ); 
		undoContainer->clear();
		canUndo = false;		

  		dr.ReadTypeSpecifierList();
		if( dr.GetTypeSpecPackage()->IsNoChildPackages() )
			throw lastLxm;
		ct->AddChildPackage( dr.GetTypeSpecPackage().Release() );	
		ct->AddChildPackage( dr.ReadNextDeclarator().Release() );						
		op->AddChildPackage(ct.Release());

		lexemContainer.insert(lexemContainer.end(), 
			dr.GetLexemContainer().begin(), dr.GetLexemContainer().end());

		op->SetPackageID( PC_CAST_OPERATOR );
		return op;
	}
		
}


// ������� ���������� ����������� �� { �� �����. �� }
void CompoundStatementReader::Read()
{
	int sc = 0;
	for( ;; )
	{
		Lexem lxm = lexicalAnalyzer.NextLexem();
		
		// ���� �� ���������� �����, ������ ��������� ������� � ����������
		if( !ignoreStream )
			lexemContainer.push_back(lxm);

		if( lxm == '{' )
			sc ++;

		else if( lxm == '}' )
		{
			sc--;
			INTERNAL_IF( sc < 0 );
			if( sc == 0 )
				break;
		}

		else if( lxm == EOF )
			theApp.Fatal(lxm.GetPos(), "����������� ����� �����");
	}
}


// �������� �������, ������� ��������� try-����
void FunctionBodyReader::ReadTryBlock()
{
	INTERNAL_IF( lexicalAnalyzer.LastLexem() != KWTRY );
	if( !ignoreStream )
		lexemContainer->push_back(lexicalAnalyzer.LastLexem());

	// ��������� ��������� �������, ��� ����� ���� ':', �����
	// ������� ������� ������ �������������. ����� ��� ������ ���� '{'
	register Lexem lxm = lexicalAnalyzer.NextLexem();
	if( lxm == ':' )
	{
		lexicalAnalyzer.BackLexem();
		ReadInitList();
		lxm = lexicalAnalyzer.LastLexem();
	}

	INTERNAL_IF( lxm != '{' );
	if( !ignoreStream )
		lexemContainer->push_back(lxm);
	

	int sc = 1;
	bool tryEnd = false;
	for( ;; )
	{
		lxm = lexicalAnalyzer.NextLexem();
		if( !ignoreStream )
			lexemContainer->push_back(lxm);

		if( lxm == '{' )
			sc++;

		else if( lxm == '}' )
		{
			sc--;
			if( sc )		// ���� ��� ��������� ������, ������ �� ���������
				continue;

			lxm = lexicalAnalyzer.NextLexem();

			// ��������� ������� ������ ���� 'catch', ���� catch-�����
			// ��� �� ����
			if( lxm == KWCATCH )
			{
				tryEnd = true;
				if( !ignoreStream )
					lexemContainer->push_back(lxm);
			}

			// ����� catch-���� ��� ������ ����
			else 
			{
				if( !tryEnd )
					theApp.Fatal(lxm.GetPos(), "try-���� ������������ �� �������� �����������");

				// ����� ���������� ������� � ����� � �������
				lexicalAnalyzer.BackLexem();
				break;
			}
		}
		
		else if( lxm == EOF )
			theApp.Fatal(lxm.GetPos(), "����������� ����� �����");
	}
}


// �������� ������� ��������� ������ �������������
void FunctionBodyReader::ReadInitList()
{
	INTERNAL_IF( lexicalAnalyzer.LastLexem() != ':' );
	if( !ignoreStream )
		lexemContainer->push_back(lexicalAnalyzer.LastLexem());

	for( ;; )
	{
		register Lexem lxm = lexicalAnalyzer.NextLexem();
		if( lxm == '{' )
			break;		

		else if( lxm == EOF )
			theApp.Fatal(lxm.GetPos(), "����������� ����� �����");

		else 
			if( !ignoreStream )
				lexemContainer->push_back(lxm);

	}
}


// ����� ���������� ����
void FunctionBodyReader::Read()
{
	// ��������� ���������� try-�����
	if( lexicalAnalyzer.NextLexem() == KWTRY )
	{
		ReadTryBlock();	
		return;
	}

	// ���� ��������� ������� ������ �������������
	else if( lexicalAnalyzer.LastLexem() == ':' )
		ReadInitList();

	int sc = 1;
	register Lexem lxm = lexicalAnalyzer.LastLexem();

	INTERNAL_IF( lxm != '{' );
	if( !ignoreStream )
		lexemContainer->push_back(lxm);

	for( ;; )
	{
		lxm = lexicalAnalyzer.NextLexem();
		
		// ���� �� ���������� �����, ������ ��������� ������� � ����������
		if( !ignoreStream )
			lexemContainer->push_back(lxm);

		if( lxm == '{' )
			sc ++;

		else if( lxm == '}' )
		{
			sc--;
			INTERNAL_IF( sc < 0 );
			if( sc == 0 )
				break;
		}

		else if( lxm == EOF )
			theApp.Fatal(lxm.GetPos(), "����������� ����� �����");
	}
}


// ��������� ������, ���������� ��������� �� ��������������.
// ����������� �-���
void InitializationListReader::ReadList( ListInitComponent *lic )
{
	// '{' - �������, ��������� �� '}'. ���� ���������� '{',
	// ����� ������� ����� ������ � ��������� � ����, ����� ���������
	// ��� � �������
	INTERNAL_IF( lic == NULL );
	for( ;; )
	{
		int lc = lexicalAnalyzer.NextLexem();
		if( lc == '}' )
			break;
		else if( lc == '{' )
		{
			ListInitComponent *subLic = 
				new ListInitComponent(lexicalAnalyzer.LastLexem().GetPos());
			ReadList(subLic);
			lic->AddInitComponent(subLic);
		}

		// ����� ��������� ���������
		else
		{
			lexicalAnalyzer.BackLexem();
			ExpressionReader er(lexicalAnalyzer, NULL, true);
			er.Read();

			// ��������� ���������			
			lic->AddInitComponent( new AtomInitComponent(
				er.GetResultOperand(), lexicalAnalyzer.LastLexem().GetPos()) );
		}

		// ���� '}', �������
		if( lexicalAnalyzer.LastLexem() == '}' )
			break;

		// ���������, ���� ��������� ������� �� ',', ������ ������
		if( lexicalAnalyzer.LastLexem() != ',' )
			throw lexicalAnalyzer.LastLexem();
	}

	// ��������� ������������ ������
	lic->Done();

	// ����������� ��������� ������� ��� ���������� �������
	lexicalAnalyzer.NextLexem();
}


// ������� ������ �������������
void InitializationListReader::Read()
{		
	listInitComponent = new ListInitComponent(lexicalAnalyzer.LastLexem().GetPos());
	ReadList( listInitComponent );	
	listInitComponent->Done();
}


// ������� ����������, ���� ���������� ����������� ������������,
// ������� ���������
void TypeExpressionReader::Read( bool fnParam, bool readTypeDecl )
{
	bool canReadExpr = true;
	DeclaratorReader dr( DV_LOCAL_DECLARATION, lexicalAnalyzer, true );

	try {
		dr.ReadTypeSpecifierList();		
		PNodePackage typeLst = dr.GetTypeSpecPackage();
		int llc = typeLst->IsNoChildPackages() ? 0 : 
			typeLst->GetLastChildPackage()->GetPackageID();
			
		// ���� �� ���� ������� ����, ��������� ���������
		if( typeLst->IsNoChildPackages() )
		{
			// ��������, ���� ���� ������� ��� ������� - ��� � ':',
			// ������ ����� �����, ���������� �������������� ������ �����
			if( lexicalAnalyzer.LastLexem() == ':' && lexicalAnalyzer.PrevLexem() == NAME )
			{
				Lexem prv = lexicalAnalyzer.PrevLexem();
				lexicalAnalyzer.NextLexem();
				throw LabelLexem( prv );
			}

			// ����� ��������� � ������ ���������
			goto read_expression;
		}

		// ������� ������ ������ ������, ���� ������� ����� ���������
		// ����� ��������� �� ���������
		else if( typeLst->GetChildPackageCount() > 1 ||
			(llc == KWCLASS || llc == KWSTRUCT || llc == KWUNION || llc == KWENUM) )
		{
			// ������� ���, ������� �������� ��� ������
			const BaseType *rbt = NULL;

			// ���������� true, ���� ���������� ���������� ���������� �� �����,
			// �.�. ���� ���������� ���� ��������������� ';'
			if( !fnParam && readTypeDecl && 
				ParserUtils::LocalTypeDeclaration( lexicalAnalyzer, &*typeLst, rbt) )
			{
				INTERNAL_IF( lexicalAnalyzer.LastLexem() != ';' );
				NodePackage *np = new NodePackage(PC_CLASS_DECLARATION);
				np->AddChildPackage( typeLst.Release() );
				resultPkg = np;

				// ���� ��� ������ �����, ��������� ���, ������������ �� ���������
				if( rbt->IsClassType() )
					redClass = static_cast<const ClassType *>(rbt);
				return ;
			}

			canReadExpr = false;
		}
	

		// ��������� ����������
		PNodePackage decl = dr.ReadNextDeclarator();

		// ���� ��� ������������, ��� ��������� �������������� �������,
		// �� ������ ���� �� ��������� �������� �������
		if( decl->FindPackage(PC_QUALIFIED_NAME) < 0 && !fnParam)
		{
			// ���� ���������� ����������� ����� ������ 1,
			// ��������� ��������� �� ����� ������
			if( decl->GetChildPackageCount() > 1 )
				canReadExpr = false;			
			throw lexicalAnalyzer.LastLexem();
		}

		// � ���� ����� ����� ���� ������ ����������,
		// ������� ��������� ������ � �������
		NodePackage *np= new NodePackage(PC_DECLARATION);
		np->AddChildPackage( typeLst.Release() );
		np->AddChildPackage( decl.Release() );

		resultPkg = np;
		if( !ignore )
			lexemContainer = new LexemContainer(dr.GetLexemContainer());

		// ��������� ������ ���������������, � ������ �������� ������
		initializatorList = dr.GetInitializatorList();
		return;
	
	} catch( const LabelLexem & ) {
		// ������������� ��������� �������, �������� ������
		throw;	
	} catch( const Lexem &lxm ) {

		// ��������� �������������� ������, �������� 'int (3)'
		// ������� ����� ������. ����� �� ��������� ��������� � 
		// ����� ��������� � ��������� ���������
		if( canReadExpr )
			goto read_expression;
		else
			throw lxm;
	}

read_expression: 
	LexemContainer *lc = const_cast<LexemContainer*>(&dr.GetLexemContainer());
	lc->insert(lc->end(), dr.GetUndoContainer().begin(), dr.GetUndoContainer().end());	
	ExpressionReader er(lexicalAnalyzer, lc, noComa, noGT, ignore);	
	er.Read();
	
	// ������� ����� � ���������� � �������� ���������������	
	resultPkg = new ExpressionPackage(er.GetResultOperand());
	if( !ignore )
		lexemContainer = er.GetLexemContainer();
}


// ���������� ��������� ������� �� ������, ���� �� ����������, ���� ��� �� ����
Lexem ExpressionReader::NextLexem()
{
	// ���� ��������� ������ �� ����, ������� ������� �� ����
	if( canUndo )
	{
		INTERNAL_IF( undoContainer == NULL );
		if( !undoContainer->empty() )
		{
			lastLxm = undoContainer->front();			
			undoContainer->pop_front();						

			// ��������� ������� � ���������� ��� �������������
			if( !ignore)
				lexemContainer->push_back(lastLxm);
			return lastLxm;
		}

		else
			canUndo = false;
	}

	lastLxm = lexicalAnalyzer.NextLexem();
	if( !ignore )
		lexemContainer->push_back(lastLxm);	
	return lastLxm;
}


// ���������� � ����� ��� � ��������� ��������� ��������� �������
void ExpressionReader::BackLexem()
{
	// ������ ���������� � ����� ������ �������
	INTERNAL_IF( lastLxm.GetCode() == 0 );

	// ���� ��������� ������ �� ����, ������� ������� � ����
	if( canUndo )	
	{
		INTERNAL_IF( undoContainer == NULL );

		if( !ignore )
			lexemContainer->pop_back();
		undoContainer->push_front(lastLxm);
	}

	else
	{
		lexicalAnalyzer.BackLexem();

		if( !ignore )
			lexemContainer->pop_back();
	}

	lastLxm = Lexem();		// ������� ������� ��� ���� ����� ����� ���������� ������
}


// ������� � undoContainer �������. ��� ���� ���� ����� ��������������
// �� �������, �� ����������
void ExpressionReader::UndoLexem( const Lexem &lxm ) 
{
	if( !ignore )
		lexemContainer->pop_back();

	undoContainer->push_front(lxm);
	canUndo = true;
}


// ��������� �������������� �������, �� �� ������ ���� �����, 
// ������������� ��������, � ����� �������� ���������, �������
// �������� ������������� ������ ������
void ExpressionReader::CheckResultOperand( POperand &pop )
{
	const Position &errPos = lexicalAnalyzer.LastLexem().GetPos();

	// ���� ��� ��� ������������� �-���
	if( pop->IsTypeOperand() || pop->IsOverloadOperand() )	
	{	
		theApp.Error(errPos, "%s �� ����� ���� ����������",
			pop->IsTypeOperand() ? "���" : "������������� �������");					
		pop = ErrorOperand::GetInstance();
	}

	// ���� �������� �������, �������� �������� �� 'this' � ������ �����
	else if( pop->IsPrimaryOperand() )
	{
		if( ExpressionMakerUtils::CheckMemberThisVisibility(pop, errPos) < 0 )
			pop = ErrorOperand::GetInstance();
	}
}


// ������� ���������
void ExpressionReader::Read()
{
	// ��������! ����� ����� ������������ �������������� ��������,
	// ���������� ������� ������ �� �������������
	NextLexem();
	EvalExpr1(resultOperand);

	// ��������, ����� �������������� ������� �� ��� �����, 
	// ������������� ��������, � ����� �������� ���������, �������
	// �������� ������������� ������ ������
	CheckResultOperand(resultOperand);
}


// ������� ����������� ����������, ������� ����� � �����. ���� 
// noError - true, ������ ��������� ������� ����������� � undoContainer
PNodePackage ExpressionReader::ReadAbstractDeclarator( bool noError, DeclarationVariant dv )
{
	bool canReadExpr = true;
	DeclaratorReader dr( dv, lexicalAnalyzer, true );

	try {
		dr.ReadTypeSpecifierList();
		PNodePackage typeLst = dr.GetTypeSpecPackage();

		// ���� �� ���� ������� ����, ��������� ��������� ������� � undoContainer
		// � �������
		if( typeLst->IsNoChildPackages() )
		{		
			UndoLexemS(dr.GetLexemContainer());
			UndoLexemS(dr.GetUndoContainer());
			if( noError )
				return NULL;
			else
				throw lexicalAnalyzer.LastLexem();
		}

		// ������� ������ ������ ������, ��������� �� ���������
		else if( typeLst->GetChildPackageCount() > 1 )
			canReadExpr = false;

		// ��������� ����������	
		PNodePackage decl = dr.ReadNextDeclarator();

		// � ���� ����� ����� ���� ������ ����������,
		// ������� ��������� ������ � �������
		NodePackage *np= new NodePackage(PC_DECLARATION);
		np->AddChildPackage( typeLst.Release() );
		np->AddChildPackage( decl.Release() );
		
		// ���� ���������� ��������� ��������� ������� � ����������,
		// ��������� �� � �����
		if( !ignore )
			lexemContainer->insert(lexemContainer->end(),
				dr.GetLexemContainer().begin(), dr.GetLexemContainer().end());		
		return np;

	} catch( const Lexem & ) {
		if( noError && canReadExpr )
			UndoLexemS( dr.GetLexemContainer() );
		else
			throw;
	}

	return PNodePackage(NULL);
}


// ������� ������ ���������, ������� ������ � ����������� ����,
// �������� ���������� ���������� �������� ������� ')'. 
// �������������� ������ ����� ���� ������
PExpressionList ExpressionReader::ReadExpressionList( )
{
	PExpressionList expList = new ExpressionList ;	
	if( lastLxm == ')' )
		return  expList;

	for( ;; )
	{
		POperand exp = NULL;
		EvalExpr2( exp );

		// ��������� ��������� ��������� �� ������������
		CheckResultOperand( exp );

		// ��������� ��������� ��������� � ������
		expList->push_back( exp );
		
		// ���� ��������� ������� - ')', ��������� ����������
		if( lastLxm == ')' )
			break;

		// ������ ��������� ������� ������ ���� ','
		if( lastLxm != ',' )
			throw lastLxm;

		INTERNAL_IF( exp.IsNull() );
		NextLexem();
	}

	return expList;
}


// ����� ���� ������ ������� ���������.
//
// �������� ','
void ExpressionReader::EvalExpr1( POperand &result )
{
	POperand temp(NULL);

	// ��� �������� ������� �������������� ������������ �����
	StackOverflowCatcher soc;	

	EvalExpr2( result );
	while( lastLxm == ',' && (!noComa || level) )
	{
		NextLexem();
		EvalExpr2(temp);

		// ������ ��������� �� ���� �������
		result = BinaryExpressionCoordinator<ComaBinaryMaker>(result, temp, 
			',', lastLxm.GetPos()).Coordinate();
	}
}


// ��������� ���������� '=', '+=','-=','*=','/=','%=','>>=','<<=','|=','&=','^=',
// throw
void ExpressionReader::EvalExpr2( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	
	int op;	

	if( lastLxm == KWTHROW )
	{
		NextLexem();

		// ���������, ���� ���� ����������, ������ throw ������,
		// � ��������� ��������� �� �����
		if( lastLxm != ';' && lastLxm != ',' && lastLxm != ':' )
			EvalExpr2(temp);	

		// ����� ������� ������� � ����� void, ��� ���� ����,
		// ��� throw ���������� ��� ���������
		else		
			temp = new PrimaryOperand( false, *new TypyziedEntity(
				(BaseType*)&ImplicitTypeManager(KWVOID).GetImplicitType(), 
				false, false, DerivedTypeList()) );
				
		// ��������� ��������� � �����
		result = UnaryExpressionCoordinator(temp, KWTHROW, lastLxm.GetPos()).Coordinate();	
	}

	else
	{

	EvalExpr3( result );
	if( (op = lastLxm) == '=' || 
		 op == PLUS_ASSIGN  || op == MINUS_ASSIGN ||
		 op == MUL_ASSIGN   || op == DIV_ASSIGN   || op == PERCENT_ASSIGN || 
		 op == LEFT_SHIFT_ASSIGN || op == RIGHT_SHIFT_ASSIGN ||
		 op == AND_ASSIGN || op == XOR_ASSIGN || op == OR_ASSIGN )
	
	{
		NextLexem();
		EvalExpr2(temp);		

		result = BinaryExpressionCoordinator<AssignBinaryMaker>(result, temp, 
			op, lastLxm.GetPos()).Coordinate();
	}

	}	// else

}


// �������� '?:'
void ExpressionReader::EvalExpr3( POperand &result )
{
	POperand temp1(NULL), temp2(NULL);
	StackOverflowCatcher soc;

	EvalExpr4( result );
	if( lastLxm == '?' )
	{
		NextLexem();
		
		level++;
		EvalExpr1(temp1);			
		level--;
		
		if( lastLxm != ':' )	
			throw lastLxm;

		NextLexem();		
		EvalExpr4(temp2);
		
		// ������ ��������� ���������
		result = TernaryExpressionCoordinator(result, temp1, temp2,
			'?', lastLxm.GetPos()).Coordinate();
	}
}


// �������� ||
void ExpressionReader::EvalExpr4( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	

	EvalExpr5( result );
	while( lastLxm == LOGIC_OR )
	{
		NextLexem();
		EvalExpr5( temp );
		
		result = BinaryExpressionCoordinator<LogicalBinaryMaker>(result, temp, 
			LOGIC_OR, lastLxm.GetPos()).Coordinate();
	}
}


// �������� &&
void ExpressionReader::EvalExpr5( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	

	EvalExpr6( result );
	while( lastLxm == LOGIC_AND )
	{
		NextLexem();
		EvalExpr6( temp );
		
		result = BinaryExpressionCoordinator<LogicalBinaryMaker>(result, temp, 
			LOGIC_AND, lastLxm.GetPos()).Coordinate();
	}
}


// �������� |
void ExpressionReader::EvalExpr6( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	

	EvalExpr7( result );
	while( lastLxm == '|' )
	{
		NextLexem();
		EvalExpr7( temp );

		result = BinaryExpressionCoordinator<IntegralBinaryMaker>(result, temp, 
			'|', lastLxm.GetPos()).Coordinate();
	}
}


// �������� '^'
void ExpressionReader::EvalExpr7( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	

	EvalExpr8( result );
	while( lastLxm == '^' )
	{
		NextLexem();
		EvalExpr8( temp );		

		result = BinaryExpressionCoordinator<IntegralBinaryMaker>(result, temp, 
				'^', lastLxm.GetPos()).Coordinate();
	}
}


// �������� '&'
void ExpressionReader::EvalExpr8( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;	

	EvalExpr9( result );
	while( lastLxm == '&' )
	{
		NextLexem();
		EvalExpr9( temp );		

		result = BinaryExpressionCoordinator<IntegralBinaryMaker>(result, temp, 
					'&', lastLxm.GetPos()).Coordinate();
	}
}

	
// ��������� ==, !=
void ExpressionReader::EvalExpr9( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;		

	EvalExpr10( result );
	while( lastLxm == EQUAL || lastLxm == NOT_EQUAL )
	{
		register int op = lastLxm;
		NextLexem();
		EvalExpr10( temp );		
			
		// ������ �������� ���������
		result = BinaryExpressionCoordinator<ConditionBinaryMaker>(result, temp, 
				op, lastLxm.GetPos()).Coordinate();
	}	
}


// ��������� <=, <, >, >=
void ExpressionReader::EvalExpr10( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;			
	register int op;

	EvalExpr11( result );
	while( (op = lastLxm) == LESS_EQU || op == GREATER_EQU || 
			op == '<' || op == '>' )
	{
		NextLexem();
		EvalExpr11( temp );		

		// ������ �������� ���������
		result = BinaryExpressionCoordinator<ConditionBinaryMaker>(result, temp, 
				op, lastLxm.GetPos()).Coordinate();
	}			
}


// ��������� <<, >>
void ExpressionReader::EvalExpr11( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;			

	EvalExpr12( result );
	while( lastLxm == RIGHT_SHIFT || lastLxm == LEFT_SHIFT )
	{
		register int op = lastLxm;
		NextLexem();
		EvalExpr12( temp );		

		// �������� �������� ������
		result = BinaryExpressionCoordinator<IntegralBinaryMaker>(result, temp, 
					op, lastLxm.GetPos()).Coordinate();
	}
}


// ��������� +, -
void ExpressionReader::EvalExpr12( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;			

	EvalExpr13( result );
	while( lastLxm == '+' || lastLxm == '-' )
	{
		register int op = lastLxm;
		NextLexem();
		EvalExpr13( temp );	
		
		// ������ �������� �������� �������� ��� ���������
		result = op == '+' 
			? 	BinaryExpressionCoordinator<PlusBinaryMaker>(result, temp, 
					op, lastLxm.GetPos()).Coordinate()
			:	BinaryExpressionCoordinator<MinusBinaryMaker>(result, temp, 
					op, lastLxm.GetPos()).Coordinate();
	}
}


// ��������� *, /, %
void ExpressionReader::EvalExpr13( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;		

	EvalExpr14( result );
	while( lastLxm == '*' || lastLxm == '/' || lastLxm == '%' )
	{
		register int op = lastLxm;

		NextLexem();
		EvalExpr14( temp );	
		
		// ������������ ���� ������������ ��������� � ���������� '%',
		// ���� �������������� � '*' ��� '/'
		result = op == '%' 
			?	BinaryExpressionCoordinator<IntegralBinaryMaker>(result, temp, 
					op, lastLxm.GetPos()).Coordinate()
			:	BinaryExpressionCoordinator<MulDivBinaryMaker>(result, temp, 
					op, lastLxm.GetPos()).Coordinate();		
	}
}


// ��������� .*, ->*
void ExpressionReader::EvalExpr14( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;			

	EvalExpr15( result );
	while( lastLxm == DOT_POINT || lastLxm == ARROW_POINT )
	{
		register int op = lastLxm;
		NextLexem();
		EvalExpr15( temp );	

		// ������������ ��������� � ��������� �� ����		
		result = BinaryExpressionCoordinator<PointerToMemberBinaryMaker>(result, temp, 
			op, lastLxm.GetPos()).Coordinate();
	}
}


// ��������� ���������� ���� '(���)'
void ExpressionReader::EvalExpr15( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;

	// �������� ����� ���������� ����
	if( lastLxm == '(' )
	{
		// ���� ����� ���, ��������� ���. ����� ���� ����� ���������
  		PNodePackage np = ReadAbstractDeclarator( true );

		// ���� ��� �� ������, ������ ����� ��������� � �������,
		// � ������� ����������� �����. lastLxm ��� ���� �� ����������
		if( np.IsNull() )
			EvalExpr16(result);

		// ����� ��������� ������� ������ ���� ')' � ����� �������� ��������
		else
		{
			lastLxm = !ignore ? lexemContainer->back() : lexicalAnalyzer.LastLexem();
			if( lastLxm != ')' )
				throw lastLxm;

			NextLexem();

			// ������ �������
			result = ExpressionMakerUtils::MakeTypeOperand(*np);
			EvalExpr15( temp );

			// ������������ ����� ���������� ����
			result = BinaryExpressionCoordinator<TypeCastBinaryMaker>(result, temp, 
				OC_CAST, lastLxm.GetPos()).Coordinate();
		}
	}

	else
		EvalExpr16(result);
}


// ������� ��������� !, ~, +, -, *, &. size, ++, --, new, delete
void ExpressionReader::EvalExpr16( POperand &result )
{
	POperand temp(NULL);
	StackOverflowCatcher soc;
	register int op;	

	if( (op = lastLxm) == '!' || op == '~' ||
		 op == '+'			  || op == '-' || 
		 op == '*'			  || op == '&' ||
		 op == INCREMENT	  || op == DECREMENT )
		 
	{
		NextLexem();
		EvalExpr15( result );
		result = UnaryExpressionCoordinator(result, 
			op == INCREMENT || op == DECREMENT ? -op : op, lastLxm.GetPos()).Coordinate();
	}

	else if( op == KWSIZEOF )
	{
		// ����� ��������� ���������������, ��������� ���, ���
		// ���������.
		NextLexem();

		// ���� ����� '(', �������� ��� ����� ���� ���
		if( lastLxm == '(' )
		{
			// ���� ����� ���, ��������� ���. ����� ���� ����� ���������
			PNodePackage np = ReadAbstractDeclarator( true );

			// ���� ��� �� ������, ������ ����� ��������� � �������,
			// � ������� ����������� �����. lastLxm ��� ���� �� ����������
			if( np.IsNull() )
				level--,				// ��������� �������, �.�. ����� ��� ��������
				EvalExpr17(result);

			// ����� ��������� ������� ������ ���� ')' � ������ ������� �������
			else
			{				
				lastLxm = !ignore ? lexemContainer->back() : lexicalAnalyzer.LastLexem();
				if( lastLxm != ')' )
					throw lastLxm;
				
				NextLexem();								
				// ������ �������
				result = ExpressionMakerUtils::MakeTypeOperand(*np);
			}			
		}
		
		// ����� ��������������� ��� � �� ��������� ������� ���������
		else
			EvalExpr15( result );

		// ������������ ��������� sizeof
		result = UnaryExpressionCoordinator(result, KWSIZEOF, lastLxm.GetPos()).Coordinate();
	}

	// ��������� ��������� ������ � ������������. ����� ����������� �����������
	// ������� ���������� � '::', �.�. ��� ����� ���� '::new' ��� '::delete'
	else if( op == KWNEW || op == KWDELETE || op == COLON_COLON )
	{
		// ��������� ��������� �������
		bool globalOp = false;
		if( op == COLON_COLON )
		{
			globalOp = true;

			Lexem temp = lastLxm;
			NextLexem();
			if( lastLxm != KWNEW && lastLxm != KWDELETE )
			{				
				UndoLexem(lastLxm);
				UndoLexem(temp);				
				NextLexem();
				EvalExpr17( result );
				return;
			}

			else
				op = lastLxm;
		}

		// ��������� ������ ����������� 'new'
		bool arrayOp = false;
		if( op == KWNEW )
		{
			// � ����� ����������� ����, ��������� 'new', ��������:
			// ������ ���������-����������, ��� ���������� ������,
			// ������ ���������������. � ������ ������, ���������
			// ���������������, �.�. � ���������� � ���, ����� ���� � 
			// �������.
			NextLexem();

			// ��������� ��� new-� �����������
			PExpressionList placementList = new ExpressionList;

			// ��� ���������� ������			
			PNodePackage allocType = NULL;

			// ������ ��������������� 
			PExpressionList initList = new ExpressionList;

			// ������� ������� ������� ����������, � ��� ����� ���������
			if( lastLxm == '(' )
			{				
				// �������� ������� ����������				
  				allocType = ReadAbstractDeclarator( true );				

				// ���� ��� �� ������, ������ �����, ������ ��������� � �������,
				// ������� ������������ ��������� ��� new-� �����������
				if( allocType.IsNull() )
				{
					NextLexem();					
					placementList = ReadExpressionList();						
					NextLexem();
		
					// ������ ��������� �� ����� ���� ������
					if( placementList->empty() )
						throw lastLxm;
				}
				
				// ����� ���������� ������ ������������ ')'
				else
				{					
					if( lexicalAnalyzer.LastLexem() != ')' )
						throw lastLxm;
					NextLexem();
					goto read_init_list;
				}
			}

			// ��� ����� ���� � �������, ������ ����� ������ ��������� � �����������,
			// ������� ������� ���������� �� �������
			if( lastLxm == '(' )
			{
				allocType = ReadAbstractDeclarator( false );	
				if( lexicalAnalyzer.LastLexem() != ')' )
					throw lastLxm;
				NextLexem();
			}

			else
			{
				BackLexem();
				allocType = ReadAbstractDeclarator( false, DV_NEW_EXPRESSION );	

				// �������� ��������� �������. ���� ������� �������� ����������
				// � �� �������� ����������� ����� ��� '(', 
				Lexem temp = lexicalAnalyzer.LastLexem();
				if( NextLexem() != '(' )
				{
					// ���� ��������� ��������� ������� ���� ������� ����� ']',
					// ��� �������� � ������ � ����� lastLxm, ������� �� �������
					// �� ���������� �����������
					if( !(temp.GetPos().col == lastLxm.GetPos().col &&
						  temp.GetPos().line == lastLxm.GetPos().line) )
						BackLexem() ;
					lastLxm = temp,
					lexicalAnalyzer.SetLastLexem( temp );
				}
			}
			
		read_init_list:
			
			// ��������� ������ ��������������� ���� ���������
			if( lastLxm == '(' )
			{
				NextLexem();
				initList = ReadExpressionList();	
				NextLexem();
			}

			// � ����� ����� ������ ���������. ������� ������, ���
			// ��������� new, ����� ��� � ��������� ������ �������, ������
			// ��� ������������ � ������� ����������, � ����� � ������
			// �������������� ���������			
			result = NewTernaryMaker( 
				(*allocType), placementList, initList,
				globalOp, lastLxm.GetPos()).Make();
		}

		// ����� ��������� 'delete'
		else
		{
			NextLexem();

			// ����� ���� ������������ �������			
			if( lastLxm == '[' )
				if( NextLexem() != ']' )
					throw lastLxm;
				else
				{
					arrayOp = true;
					NextLexem();
				}

			// ��������� ��������� ����������
			EvalExpr15(result);

			// ������������ �������� ������������ ������. ���� ���������� ����������
			// ��������, ���� ���������� ������������
			int opc = (arrayOp ? OC_DELETE_ARRAY : KWDELETE);
			result = UnaryExpressionCoordinator(result, globalOp ? -opc : opc,
				lastLxm.GetPos()).Coordinate();
		}
	}
	
	else	
		EvalExpr17( result );		
}


// ��������� '()', '[]', '->', '.', ����������� ++, --,
// dynamic_cast, static_cast, const_cast, reinterpret_cast, typeid
void ExpressionReader::EvalExpr17( POperand &result )
{		
	POperand temp(NULL);
	StackOverflowCatcher soc;
	int op;

	// ����� ���������� ����
	if( lastLxm == KWDYNAMIC_CAST || lastLxm == KWSTATIC_CAST ||
		lastLxm == KWREINTERPRET_CAST || lastLxm == KWCONST_CAST )
	{
		op = lastLxm;

		// ����� ������ ���� '<' ��� '>'
		if( NextLexem() != '<' )
			throw lastLxm;

		// ��������� ���		
		PNodePackage np = ReadAbstractDeclarator( false );
		lastLxm = !ignore ? lexemContainer->back() : lexicalAnalyzer.LastLexem();
		
		// ��� ������ ������������� �� '>', ������ ������ ���� 
		// ��������� � �������
		if( lastLxm != '>' )
			throw lastLxm;

		if( NextLexem() != '(' )
			throw lastLxm;

		// ��������� ���������
		level++;
		NextLexem();
		EvalExpr1( temp );	
		level--;

		// ������ ������
		if( lastLxm != ')' )
			throw lastLxm;

		// ������� ������� �������, �� ������ ���������� ������
		INTERNAL_IF( np.IsNull());
		result = ExpressionMakerUtils::MakeTypeOperand(*np);

		// ������������ ���������� ����
		switch(op)
		{
		case KWDYNAMIC_CAST: 
			result = BinaryExpressionCoordinator<DynamicCastBinaryMaker>(
								result, temp, op, lastLxm.GetPos()).Coordinate();
			break;

		case KWSTATIC_CAST:
			result = BinaryExpressionCoordinator<StaticCastBinaryMaker>(
								result, temp, op, lastLxm.GetPos()).Coordinate();
			break;

		case KWREINTERPRET_CAST:
			result = BinaryExpressionCoordinator<ReinterpretCastBinaryMaker>(
								result, temp, op, lastLxm.GetPos()).Coordinate();
			break;

		case KWCONST_CAST:
			result = BinaryExpressionCoordinator<ConstCastBinaryMaker>(
								result, temp, op, lastLxm.GetPos()).Coordinate();		
			break;
		default:
			INTERNAL( "����������� ��� ���������" );
		}

		NextLexem();
	}

	// �������� typeid
	else if( lastLxm == KWTYPEID )
	{
		// typeid ( <��� ��� ���������> )
		if( NextLexem() != '(' )
			throw lastLxm;

		// �������� ������� ������� ���
  		PNodePackage np = ReadAbstractDeclarator( true );

		// ���� ��� �� ������, ������ ����� ��������� � �������,
		// � ������� ����������� �����. lastLxm ��� ���� �� ����������
		if( np.IsNull() )					
			EvalExpr18(result);		

		// ����� ��������� ������� ������ ���� ')' � ����� �������� ��������
		else
		{
			lastLxm = !ignore ? lexemContainer->back() : lexicalAnalyzer.LastLexem();
			if( lastLxm != ')' )
				throw lastLxm;
			NextLexem();
			// ������ ������� �������
			result = ExpressionMakerUtils::MakeTypeOperand(*np);		
		}
		
		// ������������ �������� typeid
		result = UnaryExpressionCoordinator(result, KWTYPEID, lastLxm.GetPos()).Coordinate();		
	}

	else
		EvalExpr18(result);

	for(;;)
	{
		if( lastLxm == '(' || lastLxm == '[' )
		{
			int b = lastLxm == '(' ? ')' : ']';
			op = lastLxm;

			NextLexem();

			if( op == '(' )
			{
				// ���������� ������ ���������� �������, ������� ����� ����
				// ������, ��������� ��� 
				PExpressionList parametrList = ReadExpressionList();
				INTERNAL_IF( lastLxm != ')' );

				// ������������ ����� �������
				result = FunctionCallBinaryMaker(result, 
					parametrList, OC_FUNCTION, lastLxm.GetPos()).Make(); 
			}

			else 
			{
				if( lastLxm == ']' )
					throw lastLxm;
				else
				{
					level++;
					EvalExpr1( temp );		// ��������� ���������-������
					level--;

					if( lastLxm != ']' )
						throw lastLxm;					
				}

				// ������������ ��������� � �������� �������
				result = BinaryExpressionCoordinator<ArrayBinaryMaker>(
					result, temp, OC_ARRAY, lastLxm.GetPos()).Coordinate();
			}
		}

		// �������� ����� ������
		else if( lastLxm == '.' || lastLxm == ARROW )
		{
			op = lastLxm;

			// ��������� ����
			INTERNAL_IF( !undoContainer->empty() );
			QualifiedConstructionReader qcr(lexicalAnalyzer, false, false);
			PNodePackage id = qcr.ReadQualifiedConstruction();
		
			// �� ����� ���� ��������� �� ����
			if( qcr.IsPointerToMember() )
				throw qcr.GetLexemContainer().back();
			
			// ��������� ��������� ������� � ����������, ���� ���������
			if( !ignore )
				lexemContainer->insert(lexemContainer->end(), 
					qcr.GetLexemContainer().begin(), qcr.GetLexemContainer().end());

			// ������ ���������
			result = SelectorBinaryMaker(result, *id, op, lastLxm.GetPos()).Make();
			
			// ���� ������ �������� ����������, � �� ���� � ������, ������
			// ��������, ������� ������� �� ������������
			if( id->GetLastChildPackage()->GetPackageID() == PC_CAST_OPERATOR && 
				qcr.GetLexemContainer().back() == ')' )
			{
				lastLxm = qcr.GetLexemContainer().back();
				continue;
			}			
		}

		// ����������� ���������, ���������
		else if( lastLxm == INCREMENT || lastLxm == DECREMENT )
		{
			// ������������ ����������� �������
			result = UnaryExpressionCoordinator(result, lastLxm, lastLxm.GetPos()).Coordinate();
		}

		else
			break;
		NextLexem();
	}
}


// �������, true, false, this,
// ������������� (������������� ��������, ����������, �������� ����������)
// ��� ��������� � �������
void ExpressionReader::EvalExpr18( POperand &result )
{
	register int tokcode = lastLxm;

	// ���� ��������� � �������
	if( tokcode == '(' )
	{
		NextLexem();
		level++;
		EvalExpr1(result);
		level--;
		if( lastLxm != ')' )		
			throw lastLxm;				

		// ������, ��� ��������� � �������
		if( !result.IsNull() && result->IsExpressionOperand() )
			static_cast<Expression &>(*result).SetCramps();
		NextLexem();
	}

	// ���� �������������
	else if( tokcode == NAME || tokcode == COLON_COLON || tokcode == KWOPERATOR )
	{		
		BackLexem();		// ���������� ������� � �����

		// undoContainer ����� ���� NULL, ��� ��� ����������
		PLexemContainer uc( undoContainer );	
		QualifiedConstructionReader qcr(lexicalAnalyzer, false, false, uc);
		PNodePackage id = qcr.ReadQualifiedConstruction();
		uc.Release();	

		// �� ����� ���� ��������� �� ����
		if( qcr.IsPointerToMember() )
			throw qcr.GetLexemContainer().back();
			
		// ��������� ��������� ������� � ����������, ���� ���������
		if( !ignore )
			lexemContainer->insert(lexemContainer->end(), qcr.GetLexemContainer().begin(),
				qcr.GetLexemContainer().end());

		// ���� ������ �������� ����������, � �� ���� � ������, ������
		// ��������, ������� ������� �� ������������
		if( id->GetLastChildPackage()->GetPackageID() == PC_CAST_OPERATOR &&
			lexicalAnalyzer.LastLexem() != '(' )
			lastLxm = qcr.GetLexemContainer().back();

		else
			NextLexem();

		// ������� �������������
		result = IdentifierOperandMaker(*id).Make();
	}


	// ���� �������
	else if( IS_LITERAL(tokcode) || tokcode == KWTRUE || tokcode == KWFALSE )
	{
		// ������� �������
		result = LiteralMaker(lastLxm).Make();
		NextLexem();
	}
	
	// ���� ��������� this
	else if( tokcode == KWTHIS )
	{
		// ������� ��������� this � ����������
		result = ThisMaker( lastLxm.GetPos() ).Make();
		NextLexem();
	}

	// ����� ���� '������� ������������ ����', ���������� ������� ����� �����
	// ������������, ������� ������������ ���������� ����
	else if( IS_SIMPLE_TYPE_SPEC(tokcode) )
	{
		// ��������� ������� ������ ���� '(', �.�. ������ ���� ����� ������������
		if( NextLexem() != '(' )
			throw lastLxm;

		// ��������� ������ ���������� � ���������� ���� � ��������� ���
		NextLexem();
		PExpressionList parametrList = ReadExpressionList();
		INTERNAL_IF( lastLxm != ')' );

		// ������� ����� ������� ����� ���
		result = FunctionCallBinaryMaker(ExpressionMakerUtils::MakeSimpleTypeOperand(tokcode),
			parametrList, OC_FUNCTION, lastLxm.GetPos()).Make();
		NextLexem();
	}

	else
		throw lastLxm;	// ������� �������������� ������
}

