/*
	Vita Development Suite Samples
*/

/* library.h : Declares the exported functions, variables and classes for the PRX */
#ifndef LIBRARY_H
#define LIBRARY_H

#ifdef LIBRARY_IMPL
#define PRX_INTERFACE  __declspec(dllexport)
#else
#define PRX_INTERFACE  __declspec(dllimport)
#endif

/*E Exported variable */
/*J エクスポートされた変数*/
PRX_INTERFACE double specialNumber;
/*E Exported function */
/*J エクスポートされた関数*/
PRX_INTERFACE double addNumbers(double a, double b);

/*E All members are exported in this class */
/*J 全てのメンバーはこのクラスにエクスポートされる*/
class PRX_INTERFACE ExportedClass {
public:
	ExportedClass() : memberVariable(2.5) {}
	~ExportedClass() {}

	double subtractNumbers(double a, double b);
	double divideNumbers(double a, double b);
	double memberVariable;
private:
	ExportedClass(ExportedClass const &);
	ExportedClass & operator=(ExportedClass const &);
};

/*E Static members can be exported too */
/*J スタティックメンバーもエクスポート可能*/
class SimpleClass {
public:
	SimpleClass() {}
	~SimpleClass() {}

	PRX_INTERFACE static double negateNumber(double a);
	static void notExported();
private:
	SimpleClass(SimpleClass const &);
	SimpleClass & operator=(SimpleClass const &);
};

#endif /* LIBRARY_H */
