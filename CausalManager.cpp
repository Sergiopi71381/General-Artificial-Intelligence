#include "StdAfx.h"
#include "MyIncludes.h"
#include "CausalManager.h"
#include "ArrayOfIntegers.h"
#include "ArrayOfArraysOfInt.h"
#include "ArrayOfCausalRelations.h"
#include "ArrayOfNeurons.h"
#include "ArrayOfStr.h"
#include "ListOfArraysOfInt.h"
#include "ListOfIntegers.h"
#include "CausalRelations.h"
#include "CStr.h"
#include "Visual.h"
#include "CausalInterlevel.h"
#include "CausalSCA.h"
#include "BlockSystem.h"
#include "TravelingSalesman.h"
#include "TSPBlock.h"
#include "HierarchicalOptimization.h"
#include "SetOfEdges.h"
#include "Services.h"
#include "CausalSet.h"
#include <math.h>
#include <stdio.h>
#include <io.h>

//--------------------------------------------------------------
CausalManager::CausalManager(){

}										//Constructor
//--------------------------------------------------------------
CausalManager::~CausalManager(void){

}										//Destructor
//--------------------------------------------------------------
void CausalManager::MenuTests(const char * text)const{
//"Text" is the text in the submenu that has been clicked.
//It could also be a button, or a form load. 
	Services::Trace(text);
}								//MenuTests
//--------------------------------------------------------------
void CausalManager::ReadAndExecuteCommands() {

	//Open the command file.
	CommandFile  = "C:\\AMine\\MSDev\\projects\\CppRefactoring5\\CausalSetFiles\\CommandFile.txt";
	Services::TraceStr(CommandFile, "The command file is: ");
	pCommandFile = fopen(CommandFile.GetString(), "r"); //open for reading only, returns NULL if can't open
	Debug::Assert(pCommandFile != NULL);

	//Read the commands and close the file
	Str strCommands;
	Services::ReadFileIntoString(pCommandFile, strCommands);
	fclose(pCommandFile);
	pCommandFile = NULL;

	//Remove comments and separate into lines.
	strCommands.RemoveBetweenMarks("/*", "*/");
	strCommands.RemoveBlankLines();
	strCommands.RemoveComments("//");
	ArrayOfStr theLines;
	int nLines = theLines.CreateBySeparatingIntoNonemptyLines(strCommands);
	Services::WriteLine("The command file containes %d lines", nLines);
	theLines.Trace("====THE COMMAND FILE:");
	Services::WriteLine("====END COMMAND FILE");

	//LIST OF COMMANDS
	//WorkingDirectory
	//OutputFile
	//Reset
	//InputCausalSet
	//CalculateConnectedComponents
	//CalculateAdjacencyLevelStructure
	//InitialPermutation
	//FindAllLegalPermutations
	//TraceAllLegalPermutations
	//SelectPermutationsOfLength
	//FindBlockSystem
	//ReportProperties
	//ReportInlinksOutlinks
	//ReportGlobalLocalIndices
	//OptimizeWithNeuronSwaps
	//OptimizeWithBlockSwaps
	//Exit

	//Read and execute the commands.
	ArrayOfStr theWords;
	ArrayOfNeurons TheNeurons;
	for (int L = 0; L < nLines; ++L) {
		Str line = theLines[L];
		theWords.CreateBySeparatingIntoNonemptyWords(line);
		Str command = theWords[0];  //All commands are one-word.
		Services::WriteLine("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~EXECUTING COMMAND: %s", command.GetString());
		if (command == "WorkingDirectory") { //Sets the working directory.
			WorkingDirectory = theWords[1]; 
			Services::TraceStr(WorkingDirectory, "The working directory is: ");
		}
		else if(command == "OutputFile") {//This will open a file to use as output.
			OutputFile = theWords[1];
			Services::TraceStr(OutputFile, "The output file is: ");
			pOutputFile = fopen(OutputFile.GetString(), "w");
			Debug::Assert(pOutputFile != NULL);
		}
		else if (command == "Reset") { //Deletes all neurons and sets an empty list.
			TheNeurons.SetEmpty();  //Not yet implemented, will assert.
		}
		else if (command == "InputCausalSet") { //Reads a causal set. Neurons are automatically numbered 0 to NbrNeurons-1.
			Str exampleName = theWords[1];
			Services::WriteLine("Input example %s", exampleName.GetString());
			CE.CreateAsSmallExample(exampleName);

			//Testing ArrayOfNeurons::CreateFromStaticExample(CE).
			//TheNeurons.CreateFromStaticExample(CE);

//			int nels = CE.GetNumberElements();
//			int nlinks = CE.GetNumberLinks();
//			const ArrayOfIntegers & pred = CE.GetPredecessors();
//			const ArrayOfIntegers & succ = CE.GetSuccessors();
//			const ArrayOfIntegers & sources = CE.GetSources();
//			TheNeurons.CreateByLearningManyCausalPairs(sources, pred, succ);
			TheNeurons.CreateFromStaticExample(CE);
//			CR.CreateFromTwoArrayRelations(nels, nlinks, pred, succ);
		}
		else if (command == "CalculateConnectedComponents") {
			TheNeurons.CalculateConnectedComponents();
			int nComp = TheNeurons.GetNumberConnectedComponents();
			Services::WriteLine("Number of Connected Components = %d", nComp);
		}
		else if (command == "CalculateAdjacencyLevelStructure") {
			ArrayOfIntegers levelsOfNeurons;
			int nLevels = TheNeurons.CalculateAdjacencyLevelStructure(levelsOfNeurons);


		}
		else if (command == "InitialPermutation") {//Used as seed for the first round of optimization.
			Str & qq = theWords[1];
			if (qq == "Sequence") {
				int size = CE.GetNumberElements();
				InitialPermutation.CreateWithSequence(size);
			}
			else if (qq == "Random") { //reorders randomly, disregard causal order.
				Debug::Assert(false); //random creates illegal permutations!
				int size = CE.GetNumberElements();
				InitialPermutation.CreateRandomly(size);
			}
			else {
				Services::WriteLine("Command InitialPermutation only takes parameters Sequence or Random.");
			}
			InitialPermutation.TraceHorizontal("Initial permutation");
		}
		else if (command == "FindAllLegalPermutations") { //Use only for small causal sets. Will also list the functional and population of macrostates.
			int nLP =  CR.FindAllLegalPermutations();
			Services::WriteLine("A total of %d legal permutations have been found and stored.", nLP);
		}
		else if (command == "TraceAllLegalPermutations") {
			int count;
			int least = CR.TracePermutationsAndTheirLength(count, "All legal permutations");
			Services::WriteLine("There are %d shortest legal permutations of length %d", count, least);
		}
		else if (command == "TracePopulationsByMacrostate") {
			CR.TracePopulationOfMacrostates();
		}
		else if (command == "SelectPermutationsOfLength"){ //Selects from MatrixOfInt AllLegalPermutations to MatrixOfInt SelectedPermutations.
			Str sz = theWords[1];  //the lengths of the permutations to be selected.
			double dd;
			long ll;
			sz.ConvertMeToNumber(dd, ll);
			int len = int(ll);
			int nSelected = CR.SelectPermutationsOfLength(len);
			Services::WriteLine("Selected and stored %d permutations of length %d", nSelected, len);
		}
		else if (command == "RandomizeTriarray") {
			TheNeurons.RandomizeTriarray();
		}
		else if (command == "ReadPermutationsFromFile"){
			Str fileName = theWords[1];  //the name of the file.
			Str path = WorkingDirectory + "\\" + fileName;
			Services::TraceStr(path, "The path=\n");
			Str theFile;
			bool bb1 = Services::ReadFileIntoString(path.GetString(), theFile);
			Debug::Assert(bb1);
			Services::TraceStr(theFile, "The permutations=\n");

			//INCOMPLETE!!!!!


		
		}
		else if(command == "TraceSelectedPermutations") {
			CR.TraceSelectedPermutations();
		}
		else if (command == "FindBlockSystem") {
			int nbrSelected = CR.GetNumberSelectedPermutations();
			if (nbrSelected <= 1)Services::WriteLine("There are only %d selected permutations. Command FindBlockSystem needs 2 or more.", nbrSelected);
			else {
				ArrayOfIntegers blockIDs;
				CR.CreateBlockSystem(blockIDs);
				//ArrayOfIntegers seed, resultingBlock;
				//int nSeed = 2; 
				//seed.Create(nSeed, 10, 8);
				//int from, to;
				//int range = CR.ExpandSeedInSelectedPermutations(seed, resultingBlock);
			}
		}
		else if (command == "ReportProperties") {
			Str report;
			TheNeurons.ShortReport(report);
			Services::TraceStr(report);
			fprintf(pOutputFile, report.GetString());
		}
		else if (command == "ReportInlinksOutlinks") {
			Str report;
			TheNeurons.ReportInlinksAndOutlinks(report);
			Services::TraceStr(report);
		}
		else if (command == "ReportGlobalLocalIndices") {
			Str report;
			TheNeurons.ReportGlobalLocalIndices(report);
			Services::TraceStr(report);
		}
		else if (command == "OptimizeWithNeuronSwaps") {
			CausalRelations CR(CE);
			ArrayOfIntegers resultingPermut;
			int initialFunctional = CR.CalculateActionFunctional(InitialPermutation);
			Str sz;
			sz = InitialPermutation.ReportHorizontal("Initial permutation=  ");
			Services::TraceStr(sz);
			CR.MinimizeFunctional(InitialPermutation, resultingPermut);
			int resultFunctional = CR.CalculateActionFunctional(resultingPermut);
			//Reports.
			sz = resultingPermut.ReportHorizontal("Resulting permutation=");
			Services::TraceStr(sz);
			ArrayOfIntegers componentOfNeurons;
			TheNeurons.GetComponentsOfNeuronsInOrder(resultingPermut, componentOfNeurons); //gives the components in the order of resultingPermut.
			sz = componentOfNeurons.ReportHorizontal("Components of neurons=");
			Services::TraceStr(sz);
			sz.Format("Initial functional=%d  Functional after optimization=%d", initialFunctional, resultFunctional);
			Services::TraceStr(sz);
			fprintf(pOutputFile, sz.GetString());
		}
		else if (command == "OptimizeWithBlockSwaps") {
			

			//report results 
		}
		else if (command == "Exit") {
			//fprintf(pOutputFile, "\n================This is text that I write to the output file");
			fflush(pOutputFile);
			fclose(pOutputFile);
			return;
		}
		else {
			Str sz;
			sz.Format("Command %s does not exist", command.GetString());
			Services::TraceStr(sz);
			Debug::Assert(false);
		}
	}
}								//ReadAndExecuteCommands
//--------------------------------------------------------------
void CausalManager::CausalManagerProceed(){

	ReadAndExecuteCommands();

///	using namespace System;
//	using namespace System::IO;
//	using namespace System::Windows::Forms;

//	Str fileName;
//	bool bb = Services::SelectFileFromDialogNew(fileName);
//	int jjj = 32;

//	Directory::SetCurrentDirectory("C:\\AMine\\MSDev\\projects\\CppRefactoring5\\CausalSetExamples\\"); //Sets the system's current directory.
//	OpenFileDialog dlg;
//	dlg.ShowDialog();
//	dlg.OpenFile(); //Opens the file selected by the user with read-only permission, as specified in the FileName property. 
//	const String ^ pp = dlg.FileName;
//	Str sz(pp);


//TESTS for files in a directory.
//Ideas for this came from:  https://msdn.microsoft.com/en-us/library/t49t9ds1.aspx
//	using namespace System::IO;
//	String^ folder = "C:\\AMine\\MSDev\\projects\\CppRefactoring5\\CausalSetExamples\\";
//	array<String^>^ files = Directory::GetFiles(folder);
//	int LL = files->Length;
	

//	for (int i = 0; i < LL; ++i) {
//		String ^ zz = files[i];
//		Str ss(zz);
//		Services::TraceStr(ss);
//	}
	
//	Console::WriteLine("--== Directories inside '{0}' ==--", folder);
//	for (int i = 0; i<dir->Length; i++)Console::WriteLine(dir[i]);
//	array<String^>^ file = Directory::GetFiles(folder);
//	Console::WriteLine("--== Files inside '{0}' ==--", folder);
//	for (int i = 0; i<file->Length; i++)Console::WriteLine(file[i]);

//This is an example of using Services::SelectFileFromDialog.
//bool Services::SelectFileFromDialog(Str & filename, const char * initialDirectory, const char * filter, const char * title)
//	Str filename;
//	const char * pDirectory = "C:\\AMine\\MSDev\\projects\\CppRefactoring5\\CausalSetExamples\\";
//	const char * pFilter = "RMC Definition (*.txt)|*.txt|All Files (*.*)|*.*";
//	const char * pTitle = "Select an example";
//	bool bb = Services::SelectFileFromDialog(filename, pDirectory, pFilter, pTitle);
	//char * sz = filename.GetString();
	//Services::TraceStr(filename, "Selected file=");


	Debug::Write("\nBEGIN  CausalManagerProceed\n");
	Str task;
	task = "SmallExamples";
//	task = "LargeExamples";
//	task = "BlockSystems";


	//BLOCK SYSTEMS=============================================
	if(task == "BlockSystems"){
	//I used this section for developing BlockSystems near end of September 2013. 
		//Contains 596 distinct permutations with least functional=314 obtained by depth-first search. 
		//Str path = "C:\\AMine\\MSDev\\projects\\CppRefactoring5\\Data\\EEBlockSystem.txt";  //European example
		Str path = "C:\\AMine\\MSDev\\projects\\CppRefactoring5\\TSPData\\EEBlockSystem.txt"; //European Example   33 neurons
		MatrixOfInt trajectories;
		bool bb = trajectories.CreateFromTextFileMI(path);
		Debug::Assert(bb);

		BlockSystem BS;
		BS.CreateFromTrajectories(trajectories);
		BS.CalculateBlockSystem(); //Finds the blocks in IA, JA notation
		BS.CalculateTrajectoriesOfBlocksNew(); //the same trajectories, but now in terms of blocks.
	}
	//LARGE EXAMPLES============================================
	else if(task == "LargeExamples"){
		//Read the data, either from a file or built-in, and create arrays Predecessors, Successors.
		//CE.SelectExample("Newton");
		//CE.SelectExample("Visual");
		CE.SelectExample("EuropeanExample"); // <==No, EuropeanExample is now a small example.
		int nE = CE.GetNumberElements();
		int nL = CE.GetNumberLinks();
		const ArrayOfIntegers & pred = CE.GetPredecessors();
		const ArrayOfIntegers & succ = CE.GetSuccessors();

		//Create matrix AllLinks and arrays NbrInlinks, NbrOutlinks, InLinks, Outlinks for each element.
		CausalRelations CR;
		CR.CreateFromTwoArrayRelations(nE, nL, pred, succ);
		CR.TraceAllRelationsAsBlock();
		int functional = CR.CalculateActionFunctional();
		Services::WriteLine("Functional for the identical permutation=%d", functional);

		//Connected Components.
		CausalInterlevel CIL(CR);
		CIL.CalculateConnectedComponents();
		int nComponents = CIL.GetNumberConnectedComponents();
		Services::WriteLine("Number of connected components=%d", nComponents);

		//TEST of CalculateCausalAdjacencyLevelStructure
		if(nComponents == 1){
			ArrayOfArraysOfInt CALS;
			CR.CalculateCausalAdjacencyLevelStructure(nComponents, CALS);
			CALS.TraceArrayOfArraysOfInt("The elements in each causal level");
			CR.TraceNumberOfLinksByElement();
		}

		//Find all legal permutations
		ArrayOfCausalRelations ACR;
		ACR.CreateWithEmptyCausalRelations(nComponents);

		Str fork;
		fork = "doLeastAction";
//		fork = "doComponents";
		if(fork == "doLeastAction"){
			ArrayOfIntegers givenPermut, resultPermut;
			givenPermut.CreateWithSequence(nE, 0);
			CR.MinimizeFunctional(givenPermut, resultPermut);
			return;
		}

		//Component 0.
		int componentIndex = 0;
		ArrayOfIntegers pred0, succ0;
		CIL.CalculateLinksForConnectedComponent(componentIndex, pred0, succ0);
		int nE0 = CIL.GetNumberElemsInComponent(componentIndex);
		int nL0 = CIL.GetNumberLinksInComponent(componentIndex);
		CausalRelations CR0;
		CR0.CreateFromTwoArrayRelations(nE0, nL0, pred0, succ0);
		CR0.TraceAllRelationsAsBlock(); 
		int functional0 = CR0.CalculateActionFunctional();
		Services::WriteLine("Component 0. nbrElems=%d  nbrLinks=%d  functional= %d", nE0, nL0, functional0);
		CausalInterlevel CIL0(CR0);
		CIL0.CalculateConnectedComponents();
		int nComponents0 = CIL0.GetNumberConnectedComponents();
		Services::WriteLine("Component 0. nbr connected components=%d", nComponents0);
		Debug::Assert(nComponents0 == 1);

		//Component 0. Minimize functional.
		ArrayOfIntegers givenPermut0, resultPermut0;
		givenPermut0.CreateWithSequence(nE0, 0);
		CR0.MinimizeFunctional(givenPermut0, resultPermut0);

		//Component 1. Minimize functional.
		componentIndex = 1;
		ArrayOfIntegers pred1, succ1;
		CIL.CalculateLinksForConnectedComponent(componentIndex, pred1, succ1);
		int nE1 = CIL.GetNumberElemsInComponent(componentIndex);
		int nL1 = CIL.GetNumberLinksInComponent(componentIndex);
		CausalRelations CR1;
		CR1.CreateFromTwoArrayRelations(nE1, nL1, pred1, succ1);
		CR1.TraceAllRelationsAsBlock(); 
		CausalInterlevel CIL1(CR1);
		CIL1.CalculateConnectedComponents();
		int nComponents1 = CIL1.GetNumberConnectedComponents();
		Services::WriteLine("Component 1. nbr connected components=%d", nComponents1);
		Debug::Assert(nComponents1 == 1);
		int functional1 = CR1.CalculateActionFunctional();
		Services::WriteLine("Component 1. nbrElems=%d  nbrLinks=%d  functional= %d", nE1, nL1, functional1);
		ArrayOfIntegers givenPermut1, resultPermut1;
		givenPermut1.CreateWithSequence(nE1, 0);
		CR1.MinimizeFunctional(givenPermut1, resultPermut1);

		//Next step: plot the points by component.  MOVE THIS CODE TO Visual!!
		//Use ComponentOfBaseElement to decide the symbol to plot each point.
		//There are 167 points. Their element numbers are 0 to 166.
		int nPoints = 167;  //VIS.GetNumberPoints(); //temporary   See also CausalExamples.cpp
		int count0 = 0;
		int count1 = 0;
		for(int e = 0; e < nPoints; ++e){
			int comp = CIL.GetComponentOfBaseElement(e);
			if(comp == 0){++count0;}
			if(comp == 1){++count1;}
			Services::WriteLine("CausalInterlevel. Point=%3d  component=%2d", e, comp);
		}
		Services::WriteLine("comp0 has %d points, comp1 has %d points", count0, count1);
	}

	//SMALL EXAMPLES============================================
	else if(task == "SmallExamples"){  //ALL THIS HAS BEEN MOVED TO  CausalExamples::ConvertFromVisual()
		CE.SelectExample("SmallExamplesSingleLineNotation");
//		CE.SelectExample("SmallExamplesMatrixCellNotation"); <== Removed in March 2017
		
		int nE = CE.GetNumberElements();
		int nL = CE.GetNumberLinks();
		const ArrayOfIntegers & pred = CE.GetPredecessors();
		const ArrayOfIntegers & succ = CE.GetSuccessors();
		CausalRelations CR;
		CR.CreateFromTwoArrayRelations(nE, nL, pred, succ);

//BEGIN TESTS FOR BLOCK SWAPPING  using my example causal set of 03/01 to 12/ 2017.
		ArrayOfIntegers permut;
		int nElems = 11;
		permut.Create(nElems, 0, 1, 3, 2, 4, 5, 7, 6, 9, 8, 10);
//		permut.Create(nElems, 0, 1, 3, 2, 5, 9, 8, 10, 4, 7, 6);
//		permut.Create(nElems, 0, 1, 3, 2, 5, 4, 7, 6, 9, 8, 10);
		CR.TracePermutationAndLength(permut);
		//permut.TraceHorizontal("Given permut=");
		bool bb1 = permut.VerifyForPermutationOfZeroBasedSequence();
		Debug::Assert(bb1);

		CausalSCA viejo(CR);
		for (int sumOfSizes = 3; sumOfSizes <= nElems; ++sumOfSizes) {
			Services::WriteLine("CausalManager Starting with sumOfSizes=%d", sumOfSizes);
			int nSwaps = viejo.SwapAllAdjacentBlocksOfSizeSum(permut, sumOfSizes);
			Services::WriteLine("CausalManager just after SwapAllAdjacentBlocksOfSizeSum:   sumOfSizes=%d  nSwaps=%d\n", sumOfSizes, nSwaps);
		}
//END TESTS FOR BLOCK SWAPPING

		CR.TraceAllRelationsAsBlock();
		int functional = CR.CalculateActionFunctional();
		Services::WriteLine("Functional for the identical permutation=%d", functional);
		//Find connected components.
		ArrayOfIntegers componentOfElement, componentOfLink;
		int nComponents = CR.CalculateConnectedComponents(componentOfElement, componentOfLink);
		Services::WriteLine("Number of connected components=%d", nComponents);

		//Find all legal permutations.
		int nbrLegalPermuts = CR.FindAllLegalPermutations(); //Newton case gives more than 6,000,000 and exceeds memory.
		Services::WriteLine("Number of legal permutations=%d", nbrLegalPermuts);

/*		//MI.TraceMatrixOfIntWithRows("ALL LEGAL PERMUTATIONS");   <== MOVE THIS CODE TO  CausalRelations
		int count = -1;
        int least = CR.TracePermutationsAndTheirLength(count, "ALL LEGAL PERMUTATIONS");
		Services::WriteLine("Least length=%d   Nbr of shortest permutations=%d", least, count); 
		int na = CR.TracePermutationsOfLength(MI, least);
		Services::WriteLine("There are %d permutations of length %d", na, least);
		int nb = CR.TracePermutationsOfLength(MI, least + 2);
		Services::WriteLine("There are %d permutations of length %d", nb, least + 2);*/

		//Minimize functional.
		//ArrayOfIntegers givenPermut, resultPermut;
		//for (int i = 0; i < 10; ++i) {
		//	givenPermut.CreateWithSequence(nE, 0);
		//	CR.MinimizeFunctional(givenPermut, resultPermut);
		//}
	}
	else{Debug::Assert(false);}

	return;
}										//CausalManagerProceed
//--------------------------------------------------------------
