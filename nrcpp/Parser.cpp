// ���������� ��������������� ����������� - Parser.cpp

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
#include "Maker.h"
#include "MemberMaker.h"
#include "Parser.h"
#include "Body.h"
#include "Coordinator.h"
#include "Reader.h"
#include "Checker.h"
#include "ExpressionMaker.h"
#include "BodyMaker.h"
#include "Translator.h"

using namespace ParserUtils;

 
// ������� ������������
int OverflowController::counter = 0;

// ������� ������������ ��� �����������
int StatementParserImpl::OverflowStackController::deep = 0;


// ��������� �� ������, � ��������� ��������� �������, ��������
// �� ����������� ������
inline static bool NeedClassParserImpl( const Lexem &lastLxm, const NodePackage *np ) ;


// ��������� �� �������������� � ��������� ��������� �������, ���������
// �� ������� ������ ����
inline static bool NeedFunctionParserImpl( LexicalAnalyzer &la, const Identifier *id );


// ��������� �� ������, � ��������� ��������� �������, ��������� �� 
// ����������� ������������
inline static bool NeedEnumReading( const Lexem &lastLxm, const NodePackage *np ) ;


// ������� ������ ������������� � ��������� ��������
inline static ListInitComponent *ReadInitList( LexicalAnalyzer &la, 
			PDeclarationMaker &pdm, const Position &errPos );


// ����� ������� ��������������� �����������
void Parser::Run()
{
	Lexem lxm;
	for( ;; )
	{
		lxm = lexicalAnalyzer.NextLexem();
		if( lxm == EOF )
		{
			// ��� ������� ��������� ������ ���� �������
			if( !GetCurrentSymbolTable().IsGlobalSymbolTable() )
				theApp.Fatal( lxm.GetPos(), "����������� ����� �����" );
			break;
		}

		// ���� ���������� ������� ���������, ���� ���������� �������� ��
		if( lxm == KWNAMESPACE )
		{
			lxm = lexicalAnalyzer.NextLexem();

			// ���������� ������� ���������
			if( lxm == '{' )
			{
				if( MakerUtils::MakeNamepsaceDeclRegion(NULL) )
					crampControl.push_back(0);
			}

			// ����� ��������� ��� ������� ���������
			else
			{
				lexicalAnalyzer.BackLexem();
				try {
					// ��������� ��� ������� ��������� � �������� ��� � ���������
					// ������� ���������
					PNodePackage nspkg = QualifiedConstructionReader(lexicalAnalyzer, 
						false, true).ReadQualifiedConstruction();
					
					Lexem nxt = lexicalAnalyzer.NextLexem() ;
						
					// �������� ���� ���������� �������� ������� ���������
					if( nxt == '=' )
					{
						MakerUtils::MakeNamespaceAlias( &*nspkg, &*QualifiedConstructionReader(
							lexicalAnalyzer, false, true).ReadQualifiedConstruction());

						if( lexicalAnalyzer.NextLexem() != ';' )
							throw lexicalAnalyzer.LastLexem();
					}

					else
					{
						if( MakerUtils::MakeNamepsaceDeclRegion(&*nspkg) )
							crampControl.push_back(0);

						if( nxt != '{' )
							SyntaxError(nxt);						
					}
			
				} catch( const Lexem &ep ) {
					SyntaxError(ep);
					IgnoreStreamWhileNotDelim(lexicalAnalyzer);
				}

			}
		}

		// using namespace 'name' ���
		// using 'name'
		else if( lxm == KWUSING )
		{
			// ������������� ������� ���������
			if( lexicalAnalyzer.NextLexem() == KWNAMESPACE )
			{
				try {
					// ��������� ��� ������� ��������� � �������� ��� � ���������
					// ������� ���������
					PNodePackage nspkg = QualifiedConstructionReader(lexicalAnalyzer, 
						false, true).ReadQualifiedConstruction();
					
					MakerUtils::MakeUsingNamespace( &*nspkg );
					if( lexicalAnalyzer.NextLexem() != ';' )
						throw lexicalAnalyzer.LastLexem();

				} catch( const Lexem &ep ) {
					SyntaxError(ep);
					IgnoreStreamWhileNotDelim(lexicalAnalyzer);				
				}
				
			}

			// ����� using-����������
			else
			{
				lexicalAnalyzer.BackLexem();
				try {
					// ��������� ��� ������� ��������� � �������� ��� � ���������
					// ������� ���������
					PNodePackage nspkg = QualifiedConstructionReader(
						lexicalAnalyzer).ReadQualifiedConstruction();
					
					MakerUtils::MakeUsingNotMember( &*nspkg );
					if( lexicalAnalyzer.NextLexem() != ';' )
						throw lexicalAnalyzer.LastLexem();

				} catch( const Lexem &ep ) {
					SyntaxError(ep);
					IgnoreStreamWhileNotDelim(lexicalAnalyzer);				
				}
				
			}
		}

		else if( lxm == '}' )
		{
			// ���������, ��� ��������� � crampControl. ���� 0,
			// ������� �� ����� ������� ���������, ����� ������ �����
			// ������������ ����������
			if( crampControl.empty() )
				SyntaxError(lxm);
			else if( crampControl.back() != 0 )
			{
				crampControl.pop_back();
				linkSpec = crampControl.empty() || crampControl.back() != 1 ? LS_CPP : LS_C;
			}

			// ����� ����������� ������� ���������
			else
			{
				INTERNAL_IF( !GetCurrentSymbolTable().IsNamespaceSymbolTable() );
				GetScopeSystem().DestroySymbolTable();
				crampControl.pop_back();
			}			
		}

		else
		{
			lexicalAnalyzer.BackLexem();
			DeclareParserImpl(lexicalAnalyzer).Parse();		
		}
	}
}


// ����� ������� ����������, ���������� �����, �� ��������� ������� 
// ����� ���������� ��� ������ ������: ��������� ������ �����, 
// ��������� ������ �������, ���� ����������, �.�. ���������� ����������������. 
// � ������ ���� ����������, ���������������� ��������� ��� ��������� ������
void DeclareParserImpl::Parse()
{	
	NodePackage *tsl = NULL, *dcl = NULL;
	DeclaratorReader dr(DV_GLOBAL_DECLARATION, lexicalAnalyzer);

	try {		
		dr.ReadTypeSpecifierList();
		tsl = dr.GetTypeSpecPackage().Release();
				
		// ���� ��������� ��������� ������� '{', � ������������� "������",
		// ������ ����� ������������ ����������, � �������
		if( lexicalAnalyzer.LastLexem() == '{' && tsl->GetLastChildPackage() && 
			tsl->GetLastChildPackage()->GetPackageID() == STRING )
			return;

		// � ���� ����� ����� ���� ����������� ������, ���� ���������
		// ��� ������ ���� ���� ������ � PC_QUALIFIED_NAME, � ���������
		// ��������� ������� - '{' ��� ':'
		if( NeedClassParserImpl(lexicalAnalyzer.LastLexem(), tsl) )
		{
			ClassParserImpl cpi(lexicalAnalyzer, tsl, ClassMember::NOT_CLASS_MEMBER);
			cpi.Parse();

			// ���������� ����������� ������, ���� ����� ��� ��������
			if( cpi.IsBuilt() )
				TranslatorUtils::TranslateClass(cpi.GetClassType());

			// ��������� �����������
			if( lexicalAnalyzer.NextLexem() == ';' )
			{
				delete tsl;
				return;
			}
			else
				lexicalAnalyzer.BackLexem();

		}

		// ����� ���� ��������� ������� ������������, ���������
		else if( NeedEnumReading(lexicalAnalyzer.LastLexem(), tsl) )
		{
			EnumParserImpl epi(lexicalAnalyzer, tsl, ClassMember::NOT_CLASS_MEMBER);
			epi.Parse();
			if( lexicalAnalyzer.NextLexem() == ';' )
			{
				delete tsl;
				return;
			}
			else
				lexicalAnalyzer.BackLexem();

		}

		// ����� ���� ���������� ������ ��� ������������
		else if( tsl->GetChildPackageCount() == 2 && 
				 tsl->GetChildPackage(1)->GetPackageID() == PC_QUALIFIED_NAME && 
				 lexicalAnalyzer.LastLexem() == ';' )
		{
			register int pc = tsl->GetChildPackage(0)->GetPackageID() ;
			lexicalAnalyzer.NextLexem();

			if( pc == KWCLASS || pc == KWSTRUCT || pc == KWUNION )
			{
				ClassTypeMaker ctm( tsl, ClassMember::NOT_CLASS_MEMBER );
				ctm.Make();
				return;
			}

			else if( pc == KWENUM )
			{
				EnumTypeMaker etm( tsl,  ClassMember::NOT_CLASS_MEMBER );
				etm.Make();			
				return;
			}
		}


		// ��������� ������ ����������
		bool firstDecl = true;
		for( ;; )
		{
			dcl = dr.ReadNextDeclarator().Release();

			// � ���������� ������ �������������� ���
			if( dcl->FindPackage(PC_QUALIFIED_NAME) < 0 )
				throw lexicalAnalyzer.LastLexem() ;

			// ����� ������� ���������� �� ������ � ��������� ��
			DeclarationCoordinator dcoord(tsl, dcl);
			PDeclarationMaker dmak = dcoord.Coordinate();
			if( !dmak.IsNull() )
			{
				dmak->Make();

				// ����� ���� �������� �� ���� �������, ������ ���� ����. �������
				// 1. ��� ������ ����������
				// 2. ������������� �������� ��������
				// 3. ��������� ��������� ������� - '{', ��� ':', try - ���� ������� �����������				
				if( firstDecl )
				{	
					if( NeedFunctionParserImpl(lexicalAnalyzer, dmak->GetIdentifier()) )
					{
						Function &fn = *const_cast<Function *>(
							static_cast<const Function *>(dmak->GetIdentifier()) );

						// ���� � ������� ��� ���� ����, ���������� ��� � �������
						if( fn.IsHaveBody() )
						{
							theApp.Error(lexicalAnalyzer.LastLexem().GetPos(), 
								"'%s' - � ������� ��� ���� ����",
								fn.GetQualifiedName().c_str());

							// ���������� ���
							lexicalAnalyzer.BackLexem();
							FunctionBodyReader(lexicalAnalyzer, true).Read();						
						}

						else
						{
							lexicalAnalyzer.BackLexem();
							FunctionParserImpl fnpi(lexicalAnalyzer, fn);
							fnpi.Parse();							
						}

						dcoord.RestoreScopeSystem();
						return;
					}

					firstDecl = false;
				}
			}

			// ������������� �������, ������ ��� ���������
			PObjectInitializator objIator = NULL;

			// ��������� �������������
			PExpressionList initList = dr.GetInitializatorList();
			if( lexicalAnalyzer.LastLexem() == '=' )				
			{
				// ���� ������ ��������������� ��� ������, �� ��� ��������������
				// ������
				if( !initList.IsNull() )
					throw lexicalAnalyzer.LastLexem();

				// ���������, ���� ��������� ������� '{',
				// ������ ��������� ������ �������������
				else if( lexicalAnalyzer.NextLexem() == '{' )
				{
					objIator = BodyMakerUtils::MakeObjectInitializator(
					  ReadInitList(lexicalAnalyzer, dmak, lexicalAnalyzer.LastLexem().GetPos()) );
					INTERNAL_IF( dmak.IsNull() );
					
					// ���������� ����������
					TranslatorUtils::TranslateDeclaration( 
						*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()),
						objIator, true);

					// ������������� �������� �������������
					dmak = NULL;
				}

				// ����� ��������� ������� ���������
				else
				{
					lexicalAnalyzer.BackLexem();
					ExpressionReader er( lexicalAnalyzer, NULL, true );
					er.Read();
					initList = new ExpressionList;
					initList->push_back(er.GetResultOperand());					
				}
			}

			// �������������� ������ ������� ��������. ������ ��������
			// ����� �������������, � ���� ������ ���������� ������������� �� ���������
			if( !dmak.IsNull() && dmak->GetIdentifier() != NULL )			
			{
				dmak->Initialize( *initList );
				// ��������� ����� ������� NULL, ���� ������������� �� �������� ��������			
				objIator = BodyMakerUtils::MakeObjectInitializator(initList, *dmak);
				// ���������� ����������
				TranslatorUtils::TranslateDeclaration( 
					*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()),
					objIator, true);
			}
						
			// �������� ���� ���������� �����, � ���� ������ ���������
			// ������������ ������� ��, �.�. � ��� ���� �������� ������� 
			// ��������� �����
			dcoord.RestoreScopeSystem();
				
			if( lexicalAnalyzer.LastLexem() != ';' &&  
				lexicalAnalyzer.LastLexem() != ',' )
				throw lexicalAnalyzer.LastLexem() ;

			if( lexicalAnalyzer.LastLexem() == ';' )
				break;

			delete dcl;
			dcl = NULL;
		}

	} catch( Lexem &lxm ) {				
		SyntaxError(lxm);
		IgnoreStreamWhileNotDelim(lexicalAnalyzer);
	}

	delete dcl;
	delete tsl;
}


// ����� ������� ����������. ���������� true, ���� ���������� ���� �������,
// ����� ���������� false
void InstructionParserImpl::Parse()
{
	// ����������, ���� ���������, ���� ����������
	TypeExpressionReader ter( lexicalAnalyzer );	
	const NodePackage *tsl = NULL;

	// ���������� ����� ���������� �������������� ��������
	try
	{	
		ter.Read();
		INTERNAL_IF( ter.GetResultPackage() == NULL );		

		// ���� ����� ���������, ������ ���� ';', � �� ���� ������ ����������
		// �������������
		if( ter.GetResultPackage()->IsExpressionPackage() )
		{		
			const ExpressionPackage &epkg = 
				static_cast<const ExpressionPackage &>(*ter.GetResultPackage());
		#if  _DEBUG
		//	cout << ExpressionPrinter(epkg.GetExpression()).GetExpressionString() << endl; 
			{
				const ClassType *thisCls = NULL;
				if( const FunctionSymbolTable *fst = GetScopeSystem().GetFunctionSymbolTable() )
					if( fst->GetParentSymbolTable().IsClassSymbolTable() )
						thisCls = &static_cast<const ClassType &>(fst->GetParentSymbolTable());
				ExpressionGenerator eg(epkg.GetExpression(), thisCls);
				eg.Generate();
				cout << eg.GetOutBuffer() << endl;
			} //*/
		#endif

			if( lexicalAnalyzer.LastLexem() != ';' )
				throw lexicalAnalyzer.LastLexem() ;

			// ������ ���������� � �������
			insList.push_back( BodyMakerUtils::ExpressionInstructionMaker(epkg.GetExpression(), 
				lexicalAnalyzer.LastLexem().GetPos()) );
			delete &epkg;
			return;
		}

		// ���� ���� ���������� ������, ��������� ��� � ������
		if( ter.GetRedClass() )
			insList.push_back( BodyMakerUtils::ClassInstructionMaker(*ter.GetRedClass(),
				lexicalAnalyzer.LastLexem().GetPos()) );

		// ����� ����� ����������
		SmartPtr<const NodePackage> rpkg = 
			static_cast<const NodePackage *>(ter.GetResultPackage());		

		// ���������, ���� ����� ���������� ����, ������ ��������� ��������� ������� ';'
		// � ���������� �����, ���������� ��� ����������
		if( rpkg->GetPackageID() == PC_CLASS_DECLARATION )
			return ;
		
		// ����� ���������, ������� �� ����������
		const NodePackage *tsl = static_cast<const NodePackage*>(rpkg->GetChildPackage(0));
		INTERNAL_IF( rpkg->GetPackageID() != PC_DECLARATION || 
				rpkg->GetChildPackageCount() != 2 );
		AutoDeclarationCoordinator dcoord(tsl, (NodePackage*)rpkg->GetChildPackage(1));
		PDeclarationMaker dmak = dcoord.Coordinate();
		// ������ ����������
		if( !dmak.IsNull() )				
			dmak->Make();

		// ������������� �������, ������ ��� ���������
		PObjectInitializator objIator = NULL;

		// ��������� �������������
		PExpressionList initList = ter.GetInitializatorList();
		if( lexicalAnalyzer.LastLexem() == '=' )				
		{
			// ���� ������ ��������������� ��� ������, �� ��� ��������������
			// ������
			if( !initList.IsNull() )
				throw lexicalAnalyzer.LastLexem();

			// ���������, ���� ��������� ������� '{',
			// ������ ��������� ������ �������������
			else if( lexicalAnalyzer.NextLexem() == '{' )
			{
				objIator = BodyMakerUtils::MakeObjectInitializator(
					ReadInitList(lexicalAnalyzer, dmak, lexicalAnalyzer.LastLexem().GetPos()) );

				// ����� ����, ��� ������������� � ������������� ��������, ������ ����������
				insList.push_back( BodyMakerUtils::DeclarationInstructionMaker( 
					*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()), objIator,
					lexicalAnalyzer.LastLexem().GetPos() )  );
	
				// ������������� �������� �������������
				dmak = NULL;
			}

			// ����� ��������� ������� ���������
			else
			{
				lexicalAnalyzer.BackLexem();
				ExpressionReader er( lexicalAnalyzer, NULL, true );
				er.Read();
				initList = new ExpressionList;
				initList->push_back(er.GetResultOperand());					
			}
		}

		// �������������� ������ ������� ��������. ������ ��������
		// ����� �������������, � ���� ������ ���������� ������������� �� ���������
		if( !dmak.IsNull() && dmak->GetIdentifier() != NULL )		
		{
			dmak->Initialize( *initList ),

			// ��������� ����� ������� NULL, ���� ������������� �� �������� ��������			
			objIator = BodyMakerUtils::MakeObjectInitializator(initList, *dmak);
		
			// ����� ����, ��� ������������� � ������������� ��������, ������ ����������
			insList.push_back( BodyMakerUtils::DeclarationInstructionMaker( 
				*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()), objIator,
				lexicalAnalyzer.LastLexem().GetPos() ) );
		}

		if( lexicalAnalyzer.LastLexem() == ';' )
			;

		// �����, ���� �� ',', ������� �������������� ������
		else if( lexicalAnalyzer.LastLexem() != ',' )
			throw lexicalAnalyzer.LastLexem() ;

		// ����� ��������� ������ ����������
		else
		for( ;; )
		{
			DeclaratorReader dr(DV_LOCAL_DECLARATION, lexicalAnalyzer);
			PNodePackage dcl = dr.ReadNextDeclarator();

			// � ���������� ������ �������������� ���
			if( dcl->FindPackage(PC_QUALIFIED_NAME) < 0 )
				throw lexicalAnalyzer.LastLexem() ;

			// ����� ������� ���������� �� ������ � ��������� ��
			AutoDeclarationCoordinator dcoord(tsl, &*dcl);
			PDeclarationMaker dmak = dcoord.Coordinate();
			if( !dmak.IsNull() )			
				dmak->Make();
			
			// ��������� �������������
			PExpressionList initList = dr.GetInitializatorList();
			if( lexicalAnalyzer.LastLexem() == '=' )				
			{
				// ���� ������ ��������������� ��� ������, �� ��� ��������������
				// ������
				if( !initList.IsNull() )
					throw lexicalAnalyzer.LastLexem();
							
				// ��������� ������ �������������
				else if( lexicalAnalyzer.NextLexem() == '{' )
				{
					objIator = BodyMakerUtils::MakeObjectInitializator(
					  ReadInitList(lexicalAnalyzer, dmak, lexicalAnalyzer.LastLexem().GetPos()) );			
		
					// ����� ����, ��� ������������� � ������������� ��������, ������ ����������
					insList.push_back( BodyMakerUtils::DeclarationInstructionMaker( 
						*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()), objIator,
						lexicalAnalyzer.LastLexem().GetPos() ) );

					// ������������� �������� �������������
					dmak = NULL;
				}

				// ����� ��������� ������� ���������			
				else
				{
					lexicalAnalyzer.BackLexem();
					ExpressionReader er( lexicalAnalyzer, NULL, true );
					er.Read();
					initList = new ExpressionList;
					initList->push_back(er.GetResultOperand());					
				}
			}

			// �������������� ������ ������� ��������. ������ ��������
			// ����� �������������, � ���� ������ ���������� ������������� �� ���������
			if( !dmak.IsNull() && dmak->GetIdentifier() != NULL )
			{
				dmak->Initialize( *initList ),
				objIator = BodyMakerUtils::MakeObjectInitializator(initList, *dmak);
				insList.push_back( BodyMakerUtils::DeclarationInstructionMaker( 
					*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()), objIator,
					lexicalAnalyzer.LastLexem().GetPos() )  );
			}
			
			// ���������, ���� ����
			if( lexicalAnalyzer.LastLexem() == ';' )
				break;

			// �����, ���� �� ',', ������� �������������� ������
			else if( lexicalAnalyzer.LastLexem() != ',' )
				throw lexicalAnalyzer.LastLexem();
		}		

	// ������������� �����
	} catch( const LabelLexem &labLxm ) {
		// ���� �� � ����� � ����� ��������� �����, �������� �� ������
		// StatementParserImpl
		if( inBlock )
			throw;

		// ����� ������������ ��� �������������� ������
		SyntaxError(labLxm);
		IgnoreStreamWhileNotDelim(lexicalAnalyzer);
		insList.clear();
	
	// ������������� � ��������� �������
	} catch( const Lexem &lxm ) {
		SyntaxError(lxm);
		IgnoreStreamWhileNotDelim(lexicalAnalyzer);
		insList.clear();		// ������� ������ � ������ �������������� ������
	}
}


// � ����������� �������� ����� �� ������� �����, ���������
// � ������� ������ ���� �����, � ����� ����������� ����������
ClassParserImpl::ClassParserImpl( LexicalAnalyzer &la, NodePackage *np, ClassMember::AS as ) 
	: lexicalAnalyzer(la), typePkg(*np)

{
	// 1. ������� �����, ���� �� ��� �� ������, � ����� ���������
	//    ��� ������������
	// 2. ��������� ������� ����, ���� ��������� ��������� �������� 
	//    ���� ':'
	// 3. ��������� ����������� ����������� ������ � ������� ������� ���������
	// 4. ��������� ������� ��������� ������, �������� �� ��������������
	//    � ���������� � ��� ����� � ���� �������� ���������	
	//
	// ���� �� ������ 1 � 3 ���� ������������� ������, ���� ������,
	// ������������ �� '{' �� ������������� �� '}'
	ClassTypeMaker ctm(np, as, GetCurrentSymbolTable(), true);
	clsType = ctm.Make();			// ����� ����� �� "����������", ����� clsType == 0
	qualifierList = ctm.GetQualifierList();
	isUnion = clsType && clsType->GetBaseTypeCode() == BaseType::BT_UNION;	

	// ��������� ������ ������ ������� �������
	if( lexicalAnalyzer.LastLexem() == ':' )
		ReadBaseClassList();

	// �������������� ������
	if( lexicalAnalyzer.LastLexem() != '{' )
		throw lexicalAnalyzer.LastLexem();

	// ���� ����� �� ��� ������, ���� ��� ����������� ����������
	// ������ ���������� ��� ���� ������
	if( clsType == NULL || !CheckerUtils::ClassDefineChecker(
				*clsType, qualifierList, GetPackagePosition(np->GetLastChildPackage()) ) 
	  )
	{
		// ���������� ���� ������ 
		CompoundStatementReader(lexicalAnalyzer, true).Read();
		wasRead = true;
	}

	// ����� ��������� ������� ��������� ������ � ����� ����,
	// ������ ���������� ������� ��������� �� ������ ����������� ���� ��� ����
	else
	{
		wasRead = false;
		curAccessSpec = clsType->GetBaseTypeCode() == BaseType::BT_CLASS ?
			ClassMember::AS_PRIVATE : ClassMember::AS_PUBLIC; 

		// ���� ������ ������� ��������� ����������, ������ �� ������
		if( !qualifierList.IsEmpty() && qualifierList[0] == 
			::GetScopeSystem().GetFirstSymbolTable() )
			qualifierList.PopFront();

		// ��������� ������� ���������
		::GetScopeSystem().PushSymbolTableList(qualifierList);

		// ��������� ��� �����
		::GetScopeSystem().MakeNewSymbolTable(clsType);
	}
}


// ������� ����� ������
void ClassParserImpl::Parse()
{
	// ���� �� ���� ������ ��� ���������� ����� ������, ��������� ���� ������,
	// � ��������� ������ ��� ��� �������
	if( wasRead )		
		return;
	
	// ��������� '{'
	INTERNAL_IF( lexicalAnalyzer.NextLexem() != '{' );			
	for( ;; )
	{
		Lexem lxm = lexicalAnalyzer.NextLexem();
		if( lxm == '}' )
			break;

		else if( lxm == KWPUBLIC || lxm == KWPROTECTED || lxm == KWPRIVATE )
		{
			if( lexicalAnalyzer.NextLexem() != ':' )
			{
				SyntaxError(lexicalAnalyzer.LastLexem());
				lexicalAnalyzer.BackLexem();
			}
							
			curAccessSpec = lxm == KWPUBLIC ? ClassMember::AS_PUBLIC
				: (lxm == KWPRIVATE ? ClassMember::AS_PRIVATE : ClassMember::AS_PROTECTED);
		}
		
		// ���� using-����������
		else if( lxm == KWUSING )
		{
			try {
				// ��������� ��� ������� ��������� � �������� ��� � ���������
				// ������� ���������
				PNodePackage nspkg = QualifiedConstructionReader(
					lexicalAnalyzer).ReadQualifiedConstruction();
				
				MakerUtils::MakeUsingMember( &*nspkg, curAccessSpec );

				// ���������, ���� ������� �������� ����������, ������
				// ��������� ��������� �������
				if( nspkg->FindPackage(PC_CAST_OPERATOR) >= 0 )
				{	
					if( lexicalAnalyzer.LastLexem() != ';' )
						throw lexicalAnalyzer.LastLexem();
				}

				else if( lexicalAnalyzer.NextLexem() != ';' )
					throw lexicalAnalyzer.LastLexem();

			} catch( const Lexem &ep ) {
				SyntaxError(ep);
				IgnoreStreamWhileNotDelim(lexicalAnalyzer);				
			}
		}

		// ���� ���������� ������� 
		else if( lxm == KWTEMPLATE )
		{
		}

		// ����� ��������� ���� ������
		else
		{
			lexicalAnalyzer.BackLexem();
			ParseMember();			
		}
	}

	// ������ ������ ����, ��� �� ��������� ��������, �.�. �� ��������
	// �� ������
	clsType->uncomplete = false;

	// ���������� ����������� �������-�����, ������� �� ������ ����,
	// �-�� �� ���������, �-�� �����������, �-��, �������� �����������
	GenerateSMF();

	// ���������� ��������� inline-�������
	LoadInlineFunctions();

	// ����������� ��� �������, ������� ���� �������� � ���� � ������������
	// � ������ ������ ��� �� �����������
	for( int i = 0; i<qualifierList.GetSymbolTableCount(); i++ )
		::GetScopeSystem().DestroySymbolTable();

	// ������� ��� ����� �� ����� �������� ���������
	::GetScopeSystem().DestroySymbolTable();

	// ��������� friend-�������, ����� ���� ��� ��������� ������� ���������
	// �������.
	LoadFriendFunctions();
}


// ������� ������ ������� ����� � ��������� �� � ������,
// ����� ��������� �� ������������
void ClassParserImpl::ReadBaseClassList()
{
	// ��������� ':'
	INTERNAL_IF( lexicalAnalyzer.NextLexem() != ':' );

	// ����� ��������� ������ ������� �������, ���� �� �������� '{',
	// ���� �� ����� �������������� ������
	for( ;; )
	{
		PNodePackage bcp = new NodePackage( PC_BASE_CLASS );
		Lexem lxm = lexicalAnalyzer.NextLexem();

		if( lxm == KWVIRTUAL )
		{
			bcp->AddChildPackage( new LexemPackage(lxm) );

			lxm = lexicalAnalyzer.NextLexem();
			if( lxm == KWPUBLIC || lxm == KWPROTECTED || lxm == KWPRIVATE )
				bcp->AddChildPackage( new LexemPackage(lxm) );
			else
				lexicalAnalyzer.BackLexem();
		}

		else if( lxm == KWPUBLIC || lxm == KWPROTECTED || lxm == KWPRIVATE )
		{
			bcp->AddChildPackage( new LexemPackage(lxm) );

			lxm = lexicalAnalyzer.NextLexem();
			if( lxm == KWVIRTUAL )
				bcp->AddChildPackage( new LexemPackage(lxm) );
			else
				lexicalAnalyzer.BackLexem();
		}

		else
			lexicalAnalyzer.BackLexem();

		// ��������� ��� ������
		QualifiedConstructionReader qcr(lexicalAnalyzer, false, true);
		bcp->AddChildPackage( qcr.ReadQualifiedConstruction().Release() );

		// ���� ����� �� ��� ������, �� ��������� ������� ����� ��� ����
		// �� �����, ������ ���������� ���������� 
		if( clsType != NULL )
		{
		
		// ������� ������� �����
		PBaseClassCharacteristic bchar = MakerUtils::MakeBaseClass( &*bcp, 
			clsType->GetBaseTypeCode() == BaseType::BT_CLASS );

		// ����� ���� NULL, ���� ��� �������� �������� ������ ���� ������
		if( bchar.IsNull() )
			;

		// ���������, ������� �� ����� � ������
		else if( clsType->baseClassList.HasBaseClass( &bchar->GetPointerToClass() ) >= 0 )
			theApp.Error( GetPackagePosition(bcp->GetLastChildPackage()),
				"'%s' - ����� ��� ����� ��� �������", 
					bchar->GetPointerToClass().GetName().c_str());

		// ����� ���������, ��� ���� ���� ����� �����������, �� � �����������
		// ����� �����������, ����� ��������� ���-�� ����������� �������
		else
		{
			clsType->AddBaseClass(bchar);
			const ClassType &bcls = bchar->GetPointerToClass();
			if( bcls.IsPolymorphic() )
				clsType->polymorphic = true;

			clsType->abstractMethodCount += bcls.abstractMethodCount;			
		}
		
		} // clsType != NULL

		// ��������� ������� ������ ���� ���� ',' ���� '{',	
		lxm = lexicalAnalyzer.NextLexem();
		if( lxm != ',' && lxm != '{' )
		{
			clsType->baseClassList.ClearBaseClassList();	// ������� ������
			throw lxm;										// �������������� ������
		}

		if( lxm == '{' )
		{
			// ���������, ���� ����� �������� ������������,
			// �� �� ����� ����� ������ ������� �������
			if( clsType && clsType->GetBaseTypeCode() == BaseType::BT_UNION &&
				!clsType->baseClassList.IsEmpty() )
			{
				theApp.Error(lxm.GetPos(), 
					"'%s' - ����������� �� ����� ����� ������� �������",
					clsType->GetName().c_str());
				 clsType->baseClassList.ClearBaseClassList();
			}

			lexicalAnalyzer.BackLexem();
			break;
		}
	}
}


// ��������� ����
void ClassParserImpl::ParseMember( )
{
	NodePackage *tsl = NULL, *dcl = NULL;
	DeclaratorReader dr(DV_CLASS_MEMBER, lexicalAnalyzer);

	try {		
		dr.ReadTypeSpecifierList();
		tsl = dr.GetTypeSpecPackage().Release();	
				
		// � ���� ����� ����� ���� ����������� ������, ���� ���������
		// ��� ������ ���� ���� ������ � PC_QUALIFIED_NAME, � ���������
		// ��������� ������� - '{' ��� ':'
		if( NeedClassParserImpl(lexicalAnalyzer.LastLexem(), tsl) )
		{
			ClassParserImpl cpi(lexicalAnalyzer, tsl, curAccessSpec);
			cpi.Parse();

			if( lexicalAnalyzer.NextLexem() == ';' )
			{
				delete tsl;
				return;
			}
			else
				lexicalAnalyzer.BackLexem();
		}

		// ����� ���� ��������� ������� ������������, ���������
		else if( NeedEnumReading(lexicalAnalyzer.LastLexem(), tsl) )
		{
			EnumParserImpl epi(lexicalAnalyzer, tsl, curAccessSpec);
			epi.Parse();
			if( lexicalAnalyzer.NextLexem() == ';' )
			{
				delete tsl;
				return;
			}
			else
				lexicalAnalyzer.BackLexem();

		}

		// ����� ���� ���������� ������ ��� ������������
		else if( tsl->GetChildPackageCount() == 2 && 
				 tsl->GetChildPackage(1)->GetPackageID() == PC_QUALIFIED_NAME && 
				 lexicalAnalyzer.LastLexem() == ';' )
		{
			register int pc = tsl->GetChildPackage(0)->GetPackageID() ;
			lexicalAnalyzer.NextLexem();

			if( pc == KWCLASS || pc == KWSTRUCT || pc == KWUNION )
			{
				ClassTypeMaker ctm( tsl, curAccessSpec );
				ctm.Make(); 
				return;
			}

			else if( pc == KWENUM )
			{
				EnumTypeMaker etm( tsl, curAccessSpec );
				etm.Make();
				return;
			}
		}

		// ����� ���� ���������� ���������� ������
		else if( tsl->GetChildPackageCount() == 3 &&
			tsl->GetChildPackage(0)->GetPackageID() == KWFRIEND &&
			(tsl->GetChildPackage(1)->GetPackageID() == KWCLASS  ||
			 tsl->GetChildPackage(1)->GetPackageID() == KWSTRUCT ||
			 tsl->GetChildPackage(1)->GetPackageID() == KWUNION) &&
			tsl->GetChildPackage(2)->GetPackageID() == PC_QUALIFIED_NAME &&
			lexicalAnalyzer.LastLexem() == ';' )
		{
			lexicalAnalyzer.NextLexem();
			MakerUtils::MakeFriendClass(tsl);
			return;
		}


		// ��������� ������ ����������
		bool firstDecl = true;
		for( ;; )
		{
			dcl = dr.ReadNextDeclarator().Release();

			// ����� ���������, ���� �������� ������� �� � ���� ��
			// � �����������, ������ �������
			if( dcl->IsNoChildPackages() && tsl->IsNoChildPackages() )
				throw lexicalAnalyzer.LastLexem();

			// ����� ������� ���������� �� ������ � ��������� ��
			MemberDeclarationCoordinator dcoord(tsl, dcl, *clsType, curAccessSpec);
			PMemberDeclarationMaker dmak = dcoord.Coordinate();

			// ��������� ������ ���� ���������
			if( !dmak.IsNull() )
			{
				dmak->Make();

				// ����� ���� �������� �� ���� �������, ������ ���� ����. �������
				// 1. ��� ������ ����������
				// 2. ������������� �������� ��������
				// 3. ��������� ��������� ������� - '{', ��� ':', try - ���� ������� �����������				
				if( firstDecl )
				{	
					if( NeedFunctionParserImpl(lexicalAnalyzer, dmak->GetIdentifier()) )
					{
						Function &fn = *const_cast<Function *>(
							static_cast<const Function *>(dmak->GetIdentifier()) );

						// ��������� ���� � ��������� � ��������� ������ ������,
						// ����� �� ��������� ����������� ������, ��������� ���� ������
						lexicalAnalyzer.BackLexem();
						FunctionBodyReader fbr(lexicalAnalyzer, false);
						fbr.Read();

						// ����� ���� ����� ���� ';'
						if( lexicalAnalyzer.NextLexem() != ';' )
							lexicalAnalyzer.BackLexem();

						// ���������
						methodBodyList.push_back( FnContainerPair(&fn, fbr.GetLexemContainer()) );
						return;
					}

					firstDecl = false;
				}


				// ���� ����� �������, �� ��� ����� ���� ������ ������
				// ������������, ���� ��� �������������, ����� ������ �������������
				if( const Method *meth = dynamic_cast<const Method *>(dmak->GetIdentifier()) )
				{				
					// ����� ����� ���� ������� ������� ������������� ��� ������					
					if( lexicalAnalyzer.LastLexem() == '=' )
					{					
						if( lexicalAnalyzer.NextLexem().GetBuf() != "0" )
							throw lexicalAnalyzer.LastLexem();

						// ��������� �������������
						dmak->Initialize( MIT_PURE_VIRTUAL, *ErrorOperand::GetInstance() );

						// ������. �����. ��������� ������
						if( meth->IsAbstract() )
							clsType->abstractMethodCount += 1;

						lexicalAnalyzer.NextLexem();
					}

					// ����� ������������� ��� ������, � ���� ������ ��������
					// ������������� ������� � ��������
					else
					{						
						dmak->Initialize( MIT_NONE, *ErrorOperand::GetInstance() );
					}
				}

				// ����� ������������� �������-����� ��� ��������� �������,
				// � ��������� ������ ������ ������
				else if( lexicalAnalyzer.LastLexem() == '=' )
				{
					ExpressionReader er(lexicalAnalyzer, NULL, true);
					er.Read();	

					// ��������� ������������� ������� �����
					if( dmak->GetIdentifier() != NULL )
						dmak->Initialize( MIT_DATA_MEMBER, *er.GetResultOperand() );
				}

				// ���� ���� ':', ������ ����� ������� ����
				else if( lexicalAnalyzer.LastLexem() == ':' )
				{
					ExpressionReader er(lexicalAnalyzer, NULL, true);
					er.Read();
					
					// ��������� �������� �������� ����
					if( dmak->GetIdentifier() != NULL )
						dmak->Initialize( MIT_BITFIELD, *er.GetResultOperand() );
				}
			}

			if( lexicalAnalyzer.LastLexem() != ';' &&  
				lexicalAnalyzer.LastLexem() != ',' )
				throw lexicalAnalyzer.LastLexem() ;

			if( lexicalAnalyzer.LastLexem() == ';' )
				break;

			delete dcl;
			dcl = NULL;
		}

	} catch( Lexem &lxm ) {				
		SyntaxError(lxm);
		IgnoreStreamWhileNotDelim(lexicalAnalyzer);
	}

	delete dcl;
	delete tsl;
}


// �� ���������� ����������� ������, ��������� inline-�������, �������
// ��������� � ����������
void ClassParserImpl::LoadInlineFunctions()
{
	// ���� ������ ������, �����
	if( methodBodyList.empty() )
		return;

	list<FnContainerPair>::iterator p = methodBodyList.begin();
	while( p != methodBodyList.end() )
	{
		Function &fn = *(*p).first;
		LexemContainer *lc = &*(*p).second;

		// ���������� ���������, ���� �-��� �������������. ����
		// ����� ���������� �-��� LoadFriendFunctions
		if( clsType->GetFriendList().FindClassFriend(&fn) >= 0 )
		{
			p++;
			continue;
		}

		if( fn.IsHaveBody() )
		{
			theApp.Error( lc->front().GetPos(),
				"'%s' - � ������ ��� ���� ����", fn.GetQualifiedName().c_str());
			p = methodBodyList.erase(p);
			continue;
		}

		lexicalAnalyzer.LoadContainer( lc );	// ��������� ���������
		FunctionParserImpl fpi( lexicalAnalyzer, fn );
		fpi.Parse();

		// ������� ����, ����� �� ������� ������ ���������� friend-�������
		p = methodBodyList.erase(p);	
	}
}


// ��������� ������������� �������, ��������� � �������� ��������� ������
void ClassParserImpl::LoadFriendFunctions()
{
	// ���� ������ ������, �����
	if( methodBodyList.empty() )
		return;

	// �������� �� ������ �������� �������
	for( list<FnContainerPair>::iterator p = methodBodyList.begin(); 
		p != methodBodyList.end(); p++ )
	{
		Function &fn = *(*p).first;
		LexemContainer *lc = &*(*p).second;

		// ������� ��������� ������ ���� �� ��
		INTERNAL_IF( &fn.GetSymbolTableEntry() != &::GetCurrentSymbolTable() );
	
		// � ������������� �-��� ����� ���� ����
		if( fn.IsHaveBody() )
		{
			theApp.Error( lc->front().GetPos(),
				"'%s' - � ������ ��� ���� ����", fn.GetQualifiedName().c_str());
			continue;
		}

		lexicalAnalyzer.LoadContainer( lc );	// ��������� ���������
		FunctionParserImpl fpi( lexicalAnalyzer, fn );
		fpi.Parse();

	}
}


// ����������, ����� � �������������, ������������ ������� ���� ��������� ������ ������
void EnumParserImpl::Parse( )
{
	EnumTypeMaker ctm(&typePkg, curAccessSpec,  true);
	enumType = ctm.Make();			// ������������ ����� �� "����������", ����� NULL
	
	// ���� ������������ �� �������, ���� ��� ����������� ������, ����
	// ��� ����������� �� � ���������� ������� ���������
	if( enumType == NULL ||
		!enumType->IsUncomplete() || 
		(!ctm.GetQualifierList().IsEmpty() &&
		 !(GetCurrentSymbolTable().IsGlobalSymbolTable() || 
		   GetCurrentSymbolTable().IsNamespaceSymbolTable()) ) )
	{
		// ������� ������
		if( enumType != NULL )
			theApp.Error(errPos, 
				"'%s' - %s",
				enumType->GetQualifiedName().c_str(),
				!enumType->IsUncomplete() ?
				"������������ ��� ����������" : 
				"������������ ������ ������������ � ���������� ������� ���������");
		FunctionBodyReader(lexicalAnalyzer, true).Read();
		return;
	}

	// ����� ��������� ��� ��������� ������������
	Lexem lxm ;
	if( lexicalAnalyzer.NextLexem() != '{' )
		throw lexicalAnalyzer.LastLexem();

	// ������ ��������
	EnumConstantList ecl;
	int lastVal = -1;
	for( ;; )
	{		
		lxm = lexicalAnalyzer.NextLexem();

		// ��������� ���������
		if( lxm == NAME )
		{
			CharString name = lxm.GetBuf();
			lxm = lexicalAnalyzer.NextLexem();

			// ��������� �������������
			PEnumConstant pec = NULL;			
			if( lxm == '=' )
			{
				ExpressionReader er( lexicalAnalyzer, NULL, true);
				er.Read();
				POperand ival = er.GetResultOperand();				
				double v;
				if( ExpressionMakerUtils::IsInterpretable(ival, v) &&
					ExpressionMakerUtils::IsIntegral(ival->GetType()) )
					lastVal = v;
				else
					theApp.Error(errPos,
						"'%s' - ���������������� �������� ������ ���� ����� ����������",
						name.c_str());
				lxm = lexicalAnalyzer.LastLexem();
			}

			else
				lastVal++;

			pec = MakerUtils::MakeEnumConstant(name, curAccessSpec, lastVal, errPos,enumType);
			if( !pec.IsNull() )
			{
				ecl.AddEnumConstant(pec);
				lastVal = pec->GetConstantValue();
			}


			// ���� '{', �������
			if( lxm == '}' )
				break;

			// ����� ������ ���� ','
			if( lxm != ',' )
				throw lxm;						
		}

		// ���� �����, �������
		else if( lxm == '}' )
			break;

		// ����� �������������� ������
		else
			throw lxm;
	}

	enumType->CompleteCreation(ecl);
}


// ����������� ��������� ������� � ����������� ���������� � 
// ������ �������� ��������� ��� ��������������. �������� �������� �����
// ���� ����� 0
FunctionParserImpl::FunctionParserImpl( LexicalAnalyzer &la, Function &fn )
		:  lexicalAnalyzer(la) 
{	
	fnBody = fn.IsClassMember() && static_cast<Method &>(fn).IsConstructor() ?
		new ConstructorFunctionBody(fn, la.LastLexem().GetPos()) : 
		new FunctionBody(fn, la.LastLexem().GetPos());

	// ���������, ���� � ������� ��� ���� ����, ������� ������
	if( fn.IsHaveBody() )
		theApp.Error(la.LastLexem().GetPos(),
			"'%s' - � ������� ��� ���� ����", fn.GetQualifiedName().c_str());
	// ������ ���� �������. ���� ������ ���������� ���� ���
	else
		fn.SetFunctionBody();	
	
	// ������ �������������� ������� ���������
	GetScopeSystem().MakeNewSymbolTable( new FunctionSymbolTable(fn, fn.GetSymbolTableEntry()) );

}


// ������� ������ ������������� ������������ � ��������� �����. ��������
void FunctionParserImpl::ReadContructorInitList( CtorInitListValidator &cilv )
{			
	for( unsigned orderNum = 1;; )
	{
		QualifiedConstructionReader qcr(lexicalAnalyzer, false, true);
		PNodePackage id = qcr.ReadQualifiedConstruction();
		
		// �� ����� ���� ��������� �� ����
		if( qcr.IsPointerToMember() )
			throw lexicalAnalyzer.LastLexem();
			
		// ������� �������������, ������ �������� �������������� �������
		// ���������, �.�. ����� ������� ��������		
		SymbolTable *st = &::GetCurrentSymbolTable();
		GetScopeSystem().DestroySymbolTable();
		POperand result = IdentifierOperandMaker(*id).Make();
		GetScopeSystem().MakeNewSymbolTable(st);

		// ��������� ������� ������ ���� '('
		Lexem lxm = lexicalAnalyzer.NextLexem();
		if( lxm != '(' )
			throw lxm;
		
		// ��������� ������ ���������
		PExpressionList initializatorList = new ExpressionList;
		if( lexicalAnalyzer.NextLexem() != ')' )			
		{
		lexicalAnalyzer.BackLexem();
		for( ;; )
		{			
			// ��������� ��������� ���������
			ExpressionReader er(lexicalAnalyzer, NULL, true);
			er.Read();

			// ��������� ���������
			initializatorList->push_back( er.GetResultOperand() );

			if( lexicalAnalyzer.LastLexem() == ')' )
				break;

			// ��������� ����� ����������� ���� ����������
			if( lexicalAnalyzer.LastLexem() != ',' )				
				throw lexicalAnalyzer.LastLexem();
		}
		}

		// ��������� ����� ������������� 
		cilv.AddInitElement(result, initializatorList,
			lexicalAnalyzer.LastLexem().GetPos(), orderNum);

		// ��������� �� '{'
		lxm = lexicalAnalyzer.NextLexem();
		if( lxm == '{' )
			break;
		
		else if( lxm != ',' )
			throw lxm;		
	}
}


// ������ ���� �������
void FunctionParserImpl::Parse()
{
	register Lexem lxm = lexicalAnalyzer.NextLexem();
	INTERNAL_IF( lxm != '{' && lxm != ':' && lxm != KWTRY );
	bool isTryRootBlock = false;
	// ���� try-����, ����� ���� ����� ���� ������ �������������
	if( lxm == KWTRY )
	{
		lxm = lexicalAnalyzer.NextLexem();
		isTryRootBlock = true;				// ������������� ����, ��� �������� - try-����
	}	

	// ���� ����� ���� ������������, ��������� ������������� ������
	if( fnBody->IsConstructorBody() )
	{
		CtorInitListValidator cilv( 
			static_cast<const ConstructorMethod &>(fnBody->GetFunction()), 
			lexicalAnalyzer.LastLexem().GetPos() );	

		// ��������� ������ ������������� ������������
		if( lxm == ':' )			
			ReadContructorInitList(cilv);		// ��������� ��������� ������� ������ ���� '{'

		// ��������� ������������� ������
		cilv.Validate();

		// ������ ������ �������������
		static_cast<ConstructorFunctionBody &>(*fnBody).
			SetConstructorInitList(cilv.GetInitElementList());
	}

	if( lexicalAnalyzer.LastLexem() != '{' )
		throw lexicalAnalyzer.LastLexem();

	// ������ �������� �����������
	if( isTryRootBlock )
		fnBody->SetBodyConstruction( 
			new TryCatchConstruction(NULL, lexicalAnalyzer.PrevLexem().GetPos()) );
	// ����� ���������
	else
		fnBody->SetBodyConstruction( 
			new CompoundConstruction(NULL, lexicalAnalyzer.PrevLexem().GetPos()) );

	// ������� ���������� �����������
	ConstructionController constructionController(fnBody->GetBodyConstruction());

	// ��������� ���������� �����
	StatementParserImpl spi(lexicalAnalyzer, constructionController, *fnBody);
	spi.Parse();

	// ����� ���� ��� ���� ������������, ��������� �������������� ��������
	PostBuildingChecks(*fnBody).DoChecks();
}


// ������� � ������� using �����������
void StatementParserImpl::ReadUsingStatement( )
{
	// using namespace 'name' ���
	// using 'name'
	try {		
		// ������������� ������� ���������
		if( la.NextLexem() == KWNAMESPACE )
		{
			// ��������� ��� ������� ��������� � �������� ��� � ���������
			// ������� ���������
			PNodePackage nspkg = QualifiedConstructionReader(la, 
					false, true).ReadQualifiedConstruction();
					
			MakerUtils::MakeUsingNamespace( &*nspkg );
			if( la.NextLexem() != ';' )
				throw la.LastLexem();		
		}

		// ����� using-����������
		else
		{
			la.BackLexem();
		
			// ��������� ��� ������� ��������� � �������� ��� � ���������
			// ������� ���������
			PNodePackage nspkg = QualifiedConstructionReader(la).ReadQualifiedConstruction();					
			MakerUtils::MakeUsingNotMember( &*nspkg );
			if( la.NextLexem() != ';' )
				throw la.LastLexem();
		}

	} catch( const Lexem &ep ) {
		SyntaxError(ep);
		IgnoreStreamWhileNotDelim(la);				
	}	
}


// ������� for �����������
ForConstruction *StatementParserImpl::ReadForConstruction( ConstructionController &cntCtrl )
{
	Position fpos = la.LastLexem().GetPos();
	if( la.NextLexem() != '(' ) 
		throw la.LastLexem();
		
	InstructionList init;
	PInstruction cond = NULL;
	POperand iter = NULL;
	if( la.NextLexem() != ';' )
	{
		// ��������� ������ �������������
		la.BackLexem();
		InstructionParserImpl ipl(la, init);
		ipl.Parse();
		init = ipl.GetInstructionList();
	}

	if( la.NextLexem() != ';' )
	{
		la.BackLexem();
		cond = ReadCondition();
		if( la.LastLexem() != ';' )
			throw la.LastLexem();
	}

	if( la.NextLexem() != ')' )
	{
		la.BackLexem();
		iter = ReadExpression();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();
	}


	return BodyMakerUtils::ForConstructionMaker( ForInitSection(init), cond, iter,  
		controller.GetCurrentConstruction(), fpos) ;
}


// ������� ��������� ��� �����������, ������� ���������
POperand StatementParserImpl::ReadExpression( bool noComa )
{
	ExpressionReader er(la, NULL, noComa);
	er.Read();
	return er.GetResultOperand();		
}

	
// ������� ����������. ����������� �������� ���������� � ��������������
// ��� ���������. ������������ � if, switch, while, for
PInstruction StatementParserImpl::ReadCondition( )
{
	TypeExpressionReader ter(la);
	ter.Read( false, false );
	Position insPos = la.LastLexem().GetPos();

	// ��������� �����,
	INTERNAL_IF( ter.GetResultPackage() == NULL );	

	// ���� ����� ���������, ������ ������ ���������
	if( ter.GetResultPackage()->IsExpressionPackage() )			
	{
		POperand exp = static_cast<const ExpressionPackage &>(
			*ter.GetResultPackage()).GetExpression();
		delete ter.GetResultPackage();
		return new ExpressionInstruction(exp, insPos);
	}

	// ����� ����� ����������, ������� ������� �������������
	else
	{
		if( la.LastLexem() != '=' )
			throw la.LastLexem();

		// ���� ������������� ����� � �������, ��� ������
		if( !ter.GetInitializatorList().IsNull() )
			theApp.Error(la.LastLexem().GetPos(), "������������� ��� �����");
		POperand iator = ReadExpression();

		// ������ ����������
		PInstruction cond = BodyMakerUtils::MakeCondition( 
			static_cast<const NodePackage &>(*ter.GetResultPackage()), iator, insPos);
		delete ter.GetResultPackage();
		return cond;
	}
}


// ������� catch-����������
PTypyziedEntity StatementParserImpl::ReadCatchDeclaration( )
{
	if( la.NextLexem() != '(' )
		throw la.LastLexem();
	
	// ��������� ���, ���� �� ������� '...'
	if( la.NextLexem() == ELLIPSES )
	{
		// ����. ������� ������ ���� ')'
		if( la.NextLexem() != ')' )
			throw la.LastLexem();
		return NULL;
	}

	else
	{
		Position dpos = la.LastLexem().GetPos();

		// ��������� ����������
		la.BackLexem();
		DeclaratorReader dr( DV_CATCH_DECLARATION, la, true );
		dr.ReadTypeSpecifierList();		
		PNodePackage typeLst = dr.GetTypeSpecPackage();
	
		// ���� �� ���� ������� ����, ������
		if( typeLst->IsNoChildPackages() )
			throw la.LastLexem();
		
		// ��������� ����������
		PNodePackage decl = dr.ReadNextDeclarator();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();

		// ������ ����������, ����������
		return CatchDeclarationMaker(*typeLst, *decl, dpos).Make();
	}	
}


// ������� ������ catch-������������
void StatementParserImpl::ReadCatchList()
{	
	for( ;; )
	{
		if( la.NextLexem() != KWCATCH )
		{
			la.BackLexem();
			break;
		}
		
		Position cpos = la.LastLexem().GetPos();
		// ������� ������� ��������� ��� catch-�����		
		MakeLST();

		// ��������� catch-����������
		PTypyziedEntity catchObj = ReadCatchDeclaration();
		if( la.NextLexem() != '{' )
			throw la.LastLexem();

		// ������ catch-�����������
		CatchConstruction *cc = CatchConstructionMaker(
			catchObj, controller.GetCurrentConstruction(), cpos).Make();
		PBodyComponent pCatch = cc;

		// ��������� catch-����, ��������� ��� � catch-�����������
		cc->AddChildComponent( ParseBlock() );

		// ��������������� ������� ��������� � ������� �����������
		GetScopeSystem().DestroySymbolTable();
		controller.SetCurrentConstruction( cc->GetParentConstruction() );
		pCatch.Release();
	}
}


// ������� ���� 
CompoundConstruction *StatementParserImpl::ParseBlock( )
{
	INTERNAL_IF( la.LastLexem() != '{' );

	// ��������� ������� �� � ������� �����������, 
	// ��� ���� ����� � ������ ������ ������������ ��
	const SymbolTable &cur = GetCurrentSymbolTable();
	Construction &parent = controller.GetCurrentConstruction();

	// ������� ��������. �����������, ������ �� �������
	CompoundConstruction *compound = 
		BodyMakerUtils::SimpleConstructionMaker<CompoundConstruction>(
			parent, la.LastLexem().GetPos() );
	controller.SetCurrentConstruction(compound);
	while( la.NextLexem() != '}' )
	{
		la.BackLexem();

		// ��� ���������� ���������� ����� ��������� 
		// �������������� ������
		try {
			compound->AddChildComponent( ParseComponent() );
		} catch( const Lexem &lxm ) {
			// ����������� ������� �����������, ����������� ������� ���������
			controller.SetCurrentConstruction(compound);
			while( &GetCurrentSymbolTable() != &cur )
			{
				GetScopeSystem().DestroySymbolTable();
				INTERNAL_IF( GetCurrentSymbolTable().IsGlobalSymbolTable() );
			}

			SyntaxError(lxm);
			IgnoreStreamWhileNotDelim(la);	
		}
	}
	
	// ��������������� ������������ �����������
	controller.SetCurrentConstruction(&parent);
	return compound;
}


// ������� ��������� �������
BodyComponent *StatementParserImpl::ParseComponent( )
{	
	using namespace BodyMakerUtils;

	// ������������ ������������ �����
	OverflowStackController osc(la);
	PBodyComponent pComp = NULL;		// ���������, ������� �����������

	// ��������� ��������� �������
	register int lcode = la.NextLexem();
	Position compPos = la.LastLexem().GetPos();

	// ������ ����������
	if( lcode == ';' )
		return SimpleComponentMaker<EmptyInstruction>(compPos);	

	// using-���������� ��� using-namespace
	else if( lcode == KWUSING )
	{
		ReadUsingStatement();

		// ������� ������ ���������� � �������� ������ using-����������		
		return SimpleComponentMaker<EmptyInstruction>(compPos);
	}

	// namespace-�����
	else if( lcode == KWNAMESPACE )	
	{
		// ��������� ��� ������� ��������� � �������� ��� � ���������
		// ������� ���������
		PNodePackage nspkg = QualifiedConstructionReader(
			la, true, true).ReadQualifiedConstruction();
					
		if( la.NextLexem() != '=' )
			throw la.LastLexem();
					
		// ����� ��������� ������� ������� ���������		
		MakerUtils::MakeNamespaceAlias( &*nspkg, &*QualifiedConstructionReader(
				la, false, true).ReadQualifiedConstruction());

		if( la.NextLexem() != ';' )
			throw la.LastLexem();	

		// ������� ������ ���������� � �������� ������ namespace-����������		
		return SimpleComponentMaker<EmptyInstruction>(compPos);
	}
	
	// case ���������:	���������
	else if( lcode == KWCASE )
	{
		POperand exp = ReadExpression( true );
		if( la.LastLexem() != ':' )
			throw la.LastLexem();

		// ��������� ����� ���������
		BodyComponent &bc = *ParseComponent( );
		return CaseLabelMaker(exp, bc, controller.GetCurrentConstruction(), compPos);
	}

	// default: ���������
	else if( lcode == KWDEFAULT )
	{
		if( la.NextLexem() != ':' )
			throw la.LastLexem();
		BodyComponent &bc = *ParseComponent( );
		return DefaultLabelMaker(bc, controller.GetCurrentConstruction(), compPos);
	}

	// asm ( ������ ) ;
	else if( lcode == KWASM )
	{
		if( la.NextLexem() != '(' ) 
			throw la.LastLexem();
		Lexem lxm = la.NextLexem();			// ��������� ��������� �������
		if( lxm != STRING || la.NextLexem() != ')' )
			throw la.LastLexem();
		if( la.NextLexem() != ';' ) 
			throw la.LastLexem();
		
		return AsmOperationMaker(lxm.GetBuf(), compPos);
	}

	// return ���������? ;
	else if( lcode == KWRETURN )
	{
		POperand exp = NULL;
		if( la.NextLexem() == ';' )
			;
		else
		{
			la.BackLexem();
			exp = ReadExpression();
			if( la.LastLexem() != ';' )
				throw la.LastLexem() ;
		}

		return ReturnOperationMaker(exp, fnBody.GetFunction(), compPos);
	}
	
	// break;
	else if( lcode == KWBREAK )
	{
		if( la.NextLexem() != ';' )
			throw la.LastLexem();
		return BreakOperationMaker( controller.GetCurrentConstruction(), compPos);			
	}

	// continue;
	else if( lcode == KWCONTINUE )
	{
		if( la.NextLexem() != ';' )
			throw la.LastLexem();
		return ContinueOperationMaker( controller.GetCurrentConstruction(), compPos);
	}
	
	// goto �����;
	else if( lcode == KWGOTO )
	{
		Lexem lxm = la.NextLexem();
		if( lxm != NAME )
			throw la.LastLexem();
		if( la.NextLexem() != ';' )
			throw la.LastLexem();
		return GotoOperationMaker( lxm.GetBuf(), fnBody, compPos );
	}
	
	// do ��������� while( ��������� ) ;
	else if( lcode == KWDO )
	{
		// ������� ������� ��������� ��� do
		MakeLST();

		// ������� do
		DoWhileConstruction *dwc = SimpleConstructionMaker<DoWhileConstruction>(
			controller.GetCurrentConstruction(), compPos);
		pComp = dwc;

		// ������ ��� �������
		controller.SetCurrentConstruction(dwc);

		// ������� ���������
		PBodyComponent doChild = ParseComponent();
		if( la.NextLexem() != KWWHILE )
			throw la.LastLexem();
		if( la.NextLexem() != '(' )
			throw la.LastLexem();
		POperand doExpr = ReadExpression();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();
		if( la.NextLexem() != ';' )
			throw la.LastLexem();

		// ������ ��������� do-�����������, ��������� ���������, 
		// ������������ �������� ���������, ��������������� ������������ �����������, 
		// ��������������� ��		
		dwc->SetCondition(doExpr);
		ValidCondition(doExpr, "do", compPos);
		dwc->AddChildComponent( doChild.Release() );
		controller.SetCurrentConstruction( dwc->GetParentConstruction() );
		GetScopeSystem().DestroySymbolTable();
		return pComp.Release();		// ���������� do-�����������

	}

	// for( �������������? ;  ���������? ; ���������? ) ���������
	else if( lcode == KWFOR )
	{
		// ������� ������� ��������� ��� �����������
		MakeLST();

		// ��������� � ������� ���� �����������
		ForConstruction *fc = ReadForConstruction(controller);
		pComp = fc;
		// ������ ��� �������
		controller.SetCurrentConstruction(fc);
		BodyComponent *forChild = ParseComponent();

		// ������������ �������� ���������, ��������������� ������������ �����������, 
		// ��������������� ��		
		fc->AddChildComponent( forChild );
		controller.SetCurrentConstruction( fc->GetParentConstruction() );
		GetScopeSystem().DestroySymbolTable();

		// ������� for-�����������
		return pComp.Release();
	}

	// while( ��������� ) ���������
	else if( lcode == KWWHILE )
	{
		// ������� ������� ��������� ��� �����������
		MakeLST();	
		if( la.NextLexem() != '(' )
			throw la.LastLexem();
		PInstruction whileCond = ReadCondition();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();

		// ������� while-�����������, ��������� ������������ ���������
		WhileConstruction *wc = ConditionConstructionMaker<WhileConstruction>(whileCond, 
			controller.GetCurrentConstruction(), compPos);
		ValidCondition(whileCond, "while");
		pComp = wc;
		// ������ ��� �������
		controller.SetCurrentConstruction(wc);
		BodyComponent *whileChild = ParseComponent();

		// ������������ �������� ���������, ��������������� ������������ �����������, 
		// ��������������� ��		
		wc->AddChildComponent( whileChild );
		controller.SetCurrentConstruction( wc->GetParentConstruction() );
		GetScopeSystem().DestroySymbolTable();

		// ������� while-�����������
		return pComp.Release();
	}

	// switch( ��������� ) ���������
	else if( lcode == KWSWITCH )
	{	
		// ������� ������� ��������� ��� �����������
		MakeLST();
		if( la.NextLexem() != '(' )
			throw la.LastLexem();
		PInstruction switchCond = ReadCondition();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();
		
		// ������� swicth-�����������, ��������� ������������ ���������
		SwitchConstruction *sc = ConditionConstructionMaker<SwitchConstruction>(switchCond, 
			controller.GetCurrentConstruction(), compPos);
		ValidCondition(switchCond, "switch", true);
		pComp = sc;
		// ������ ��� �������
		controller.SetCurrentConstruction(sc);
		BodyComponent *switchChild = ParseComponent();	

		// ������������ �������� ���������, ��������������� ������������ �����������, 
		// ��������������� ��		
		sc->AddChildComponent( switchChild );
		controller.SetCurrentConstruction( sc->GetParentConstruction() );
		GetScopeSystem().DestroySymbolTable();

		// ������� swicth-�����������
		return pComp.Release();
	}

	// if( ��������� ) ��������� [else ���������]?
	else if( lcode == KWIF )
	{
		// ������� ������� ��������� ��� �����������
		MakeLST();	
		if( la.NextLexem() != '(' )
			throw la.LastLexem();
		PInstruction ifCond = ReadCondition();
		if( la.LastLexem() != ')' )
			throw la.LastLexem();

		// ������� if-�����������, ��������� ������������ ���������
		IfConstruction *ic = ConditionConstructionMaker<IfConstruction>(ifCond, 
			controller.GetCurrentConstruction(), compPos);
		ValidCondition(ifCond, "if");
		pComp = ic;
		// ������ ��� �������
		controller.SetCurrentConstruction(ic);
		BodyComponent *ifChild = ParseComponent();			
	
		// ������������ �������� ���������
		ic->AddChildComponent( ifChild );
		
		// ���� ���� else
		if( la.NextLexem() == KWELSE )
		{
			ElseConstruction *ec = SimpleConstructionMaker<ElseConstruction>(
					const_cast<Construction&>(*ic->GetParentConstruction()), compPos);
			PBodyComponent pElse = ec;
			// ������ ��� �������
			controller.SetCurrentConstruction(ic);
			BodyComponent *elseChild = ParseComponent();

			// ������������ �������� ���������, ������������ � if
			ec->AddChildComponent(elseChild);
			ic->SetElseConstruction( ec );

			// ����������� ���������������� ��������� ������� ������ else
			pElse.Release();				
		}
		
		else
			la.BackLexem();

		// ��������������� ������������ �����������, ��������������� ��				
		controller.SetCurrentConstruction( ic->GetParentConstruction() );
		GetScopeSystem().DestroySymbolTable();

		// ������� if-�����������
		return pComp.Release();
	}

	// try ���� ������-������������
	else if( lcode == KWTRY )
	{
		if( la.NextLexem() != '{' )
			throw la.LastLexem();

		// ������� try �����������
		TryCatchConstruction *tcc = SimpleConstructionMaker<TryCatchConstruction>(
					controller.GetCurrentConstruction(), compPos);
		pComp = tcc;

		// ������ ��� �������, ��������� ����, ��������� ����
		controller.SetCurrentConstruction(tcc);
		BodyComponent *block = ParseBlock();		
		tcc->AddChildComponent(block);

		if( la.NextLexem() != KWCATCH )		
			theApp.Error(la.LastLexem().GetPos(), "try ��� ������������");
		else	
		{
			la.BackLexem();
			ReadCatchList();
		}

		// ��������������� ������������ �����������, ���������� try-����
		controller.SetCurrentConstruction( tcc->GetParentConstruction() );
		return pComp.Release();
	}

	// ����
	else if( lcode == '{' )
	{
		bool isMade = BodyMakerUtils::MakeLocalSymbolTable( controller.GetCurrentConstruction() );
		CompoundConstruction *cc = ParseBlock();
		if( isMade )
			GetScopeSystem().DestroySymbolTable();
		return cc;
	}

	// ���� '}', ��������� ��������� ����������
	else if( lcode == '}' )
		theApp.Fatal( la.LastLexem().GetPos(), "��������� ���������� ���������� ����� '}'" );

	// ����� �����
	else if( lcode == EOF )
		theApp.Fatal( la.LastLexem().GetPos(), "����������� ����� �����" );

	// ����������: ���������, ����������, ���������� ������, �����
	else
	{
		la.BackLexem();
		InstructionList insList;		// �������� ������ ����������
		InstructionParserImpl ipl(la, insList, true);

		// ����� ����������� �����
		try {
			ipl.Parse();
			return InstructionListMaker( ipl.GetInstructionList(), compPos );
		} catch( const LabelLexem &labLxm ) {
			// ������� �����
			Label label( labLxm.GetBuf(), 
				const_cast<FunctionSymbolTable *>(GetScopeSystem().GetFunctionSymbolTable()), 
				labLxm.GetPos() );

			// ����������� �����, ��������� ���������
			return SimpleLabelMaker(label, *ParseComponent(), fnBody, compPos);
		}
	}

	// ������ ������, ���� ������� �� ����
	INTERNAL("'StatementParserImpl::ParseComponent' - ������ � ������������� ����������");
	return NULL;
}

// ������� ���� ��� try-����, � ����������� �� �������� �����������
void StatementParserImpl::Parse() {
	// ������ ��������� ����, � �� ��� catch-�����������
	if( controller.GetCurrentConstruction().GetConstructionID() == Construction::CC_TRY )
	{
		TryCatchConstruction &tcc = static_cast<TryCatchConstruction &>(
			controller.GetCurrentConstruction());

		// ��������� ����
		BodyComponent *block = ParseBlock();		
		tcc.AddChildComponent(block);

		if( la.NextLexem() != KWCATCH )		
			theApp.Error(la.LastLexem().GetPos(), "try ��� ������������");
		else	
		{
			la.BackLexem();
			try {
				ReadCatchList();
			} catch( const Lexem &lxm ) {
				theApp.Fatal(lxm.GetPos(), "��������� ���������� ���������� ����� '%s'",
					lxm.GetBuf().c_str());
			}
		}		
	}

	// ����� ��������� ������ ����
	else
		ParseBlock();
}


// ������� �������������� ������ ������������ ����� � �������� 'lxm'
void ParserUtils::SyntaxError( const Lexem &lxm )
{
	theApp.Error(
		lxm.GetPos(), "�������������� ������ ����� '%s'", lxm.GetBuf().c_str());
}


// ������� ��������� ������� �� ��� ��� ���� �� �������� ';'
void ParserUtils::IgnoreStreamWhileNotDelim( LexicalAnalyzer &la )
{
	Lexem lxm;
	for( ;; )
	{
		lxm = la.NextLexem();
		if( lxm == EOF )
			theApp.Fatal( lxm.GetPos(), "����������� ����� �����" );

		else if( lxm == '{' || lxm == '}' )
			theApp.Fatal( lxm.GetPos(), "��������� ���������� ���������� ����� '%c'",
			(char)lxm);

		else if( lxm == ';' )
			break;
	}
}


// �������� ������� ������
Position ParserUtils::GetPackagePosition( const Package *pkg )
{
	INTERNAL_IF( pkg == NULL );
	for( ;; )
	{
		if( pkg->IsLexemPackage() )
			return ((LexemPackage *)pkg)->GetLexem().GetPos();

		else if( pkg->IsNodePackage() )
		{
			NodePackage *np = (NodePackage *)pkg;
			if( np->IsNoChildPackages() )
				INTERNAL( "'ParserUtils::GetPackagePosition' - ��� �������� �������" );
			else
				pkg = np->GetChildPackage(0);
		}

		else
			INTERNAL( "'ParserUtils::GetPackagePosition' - ����������� ��� ������" );
	}

	return Position();
}


// ����������� ��� ������ �������, ������� �����
CharString ParserUtils::PrintPackageTree( const NodePackage *np )
{
	string outbuf;

	INTERNAL_IF( np == NULL );
	for( int i = 0; i<np->GetChildPackageCount(); i++ )
	{
		const Package *pkg = np->GetChildPackage(i);
		if( pkg->IsLexemPackage() )
			outbuf += ((LexemPackage *)pkg)->GetLexem().GetBuf().c_str();

		else if( pkg->IsNodePackage() )
			outbuf += PrintPackageTree( (NodePackage *)pkg ).c_str();

		else if( pkg->IsExpressionPackage() )
			;
		
		else
			INTERNAL( "'ParserUtils::GetPackagePosition' - ����������� ��� ������" );
	}

	return outbuf.c_str();
}


// ���������, ���� ������� ����� � ����. ��������� � ���������
// ���������� ���� (�����, ������������), ������� ���, ���������
// ���������� �������. ���� �� ��������� ��������� �����������, ������� true, ����� false
bool ParserUtils::LocalTypeDeclaration( 
		LexicalAnalyzer &lexicalAnalyzer, NodePackage *pkg, const BaseType *&outbt )
{
	// � ���� ����� ����� ���� ����������� ������, ���� ���������
	// ��� ������ ���� ���� ������ � PC_QUALIFIED_NAME, � ���������
	// ��������� ������� - '{' ��� ':'
	if( NeedClassParserImpl(lexicalAnalyzer.LastLexem(), pkg) )
	{
		ClassParserImpl cpi(lexicalAnalyzer, pkg, ClassMember::NOT_CLASS_MEMBER);
		cpi.Parse();
		outbt = &cpi.GetClassType();

		if( lexicalAnalyzer.NextLexem() == ';' )
			return true;			
		else
			lexicalAnalyzer.BackLexem();

	}

	// ����� ���� ��������� ������� ������������, ���������
	else if( NeedEnumReading(lexicalAnalyzer.LastLexem(), pkg) )
	{
		EnumParserImpl epi(lexicalAnalyzer, pkg, ClassMember::NOT_CLASS_MEMBER);
		epi.Parse();
		outbt = &epi.GetEnumType();

		if( lexicalAnalyzer.NextLexem() == ';' )
			return true;
		else
			lexicalAnalyzer.BackLexem();
	}

	// ����� ���� ���������� ������ ��� ������������
	else if( pkg->GetChildPackageCount() == 2 && 
			 pkg->GetChildPackage(1)->GetPackageID() == PC_QUALIFIED_NAME && 
			 lexicalAnalyzer.LastLexem() == ';' )
	{
		register int pc = pkg->GetChildPackage(0)->GetPackageID() ;
		lexicalAnalyzer.NextLexem();

		if( pc == KWCLASS || pc == KWSTRUCT || pc == KWUNION )
		{
			ClassTypeMaker ctm( pkg, ClassMember::NOT_CLASS_MEMBER );
			outbt = ctm.Make();
		}	

		else if( pc == KWENUM )
		{
			EnumTypeMaker etm( pkg,  ClassMember::NOT_CLASS_MEMBER );
			outbt = etm.Make();				
		}

		else
			return false;
		return true;
	}
	
	return false;
}


// ��������� �� �������������� � ��������� ��������� �������, ���������
// �� ������� ������ ����
inline static bool NeedFunctionParserImpl( LexicalAnalyzer &la, const Identifier *id )
{
	const Function *fn = dynamic_cast<const Function *>(id);
	register Lexem lastLxm = la.LastLexem();
	
	if( (lastLxm == '{' ||
		(dynamic_cast<const ConstructorMethod *>(fn) &&
		 (lastLxm == ':' || lastLxm == KWTRY)) ) && fn != NULL )
		return true;	

	return false;		
}


// ��������� �� ������, � ��������� ��������� �������, ��������
// �� ����������� ������
inline static bool NeedClassParserImpl( const Lexem &lastLxm, const NodePackage *np ) 
{
	if( np->IsNoChildPackages() )
		return false;

	int lid = np->GetLastChildPackage()->GetPackageID();
	if( lid == KWCLASS || lid == KWSTRUCT || lid == KWUNION )	// ���������� ����������
	{
		if( lastLxm == '{' || 
			lastLxm == ':' )
			return true;
		return false;
	}

	if( lid != PC_QUALIFIED_NAME || np->GetChildPackageCount() < 2 )
		return false;

	int preid = np->GetChildPackage(np->GetChildPackageCount()-2)->GetPackageID(),
		lcode = lastLxm;

	return (preid == KWCLASS || preid == KWSTRUCT || preid == KWUNION) &&
			(lcode == '{' || lcode == ':') ;
}


// ��������� �� ������, � ��������� ��������� �������, ��������� �� 
// ����������� ������������
inline static bool NeedEnumReading( const Lexem &lastLxm, const NodePackage *np ) 
{
	if( np->IsNoChildPackages() )
		return false;

	int lid = np->GetLastChildPackage()->GetPackageID();
	if( lid == KWENUM )	// ���������� ����������
		return lastLxm == '{';

	if( lid != PC_QUALIFIED_NAME || np->GetChildPackageCount() < 2 )
		return false;

	int preid = np->GetChildPackage(np->GetChildPackageCount()-2)->GetPackageID(),
		lcode = lastLxm;

	return (preid == KWENUM && lcode == '{');
}


// ������� ������ ������������� � ��������� ��������
inline static ListInitComponent *ReadInitList( LexicalAnalyzer &la, 
			PDeclarationMaker &pdm, const Position &errPos )
{
	// ���������
	InitializationListReader ilr(la);
	ilr.Read();
	INTERNAL_IF( ilr.GetListInitComponent() == NULL );

	// ���������, ����� ������������� ��� ��������
	
	if( pdm.IsNull() || pdm->GetIdentifier() == NULL )
		return const_cast<ListInitComponent *>(ilr.GetListInitComponent());
	   
	else if( const Object *ob = dynamic_cast<const ::Object *>(pdm->GetIdentifier()) )
	{
		ListInitializationValidator( *ilr.GetListInitComponent(), 
			errPos, const_cast<::Object &>(*ob) ).Validate();

		// ������� ������ ��� ���������
		return const_cast<ListInitComponent *>(ilr.GetListInitComponent());
	}

	// ����� ��������� ������������� �������
	else
	{
		theApp.Error(errPos, "������������� ������� ��-�������");
		return const_cast<ListInitComponent *>(ilr.GetListInitComponent());
	}
}
