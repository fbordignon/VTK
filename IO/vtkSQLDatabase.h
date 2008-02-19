/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkSQLDatabase.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
  Copyright (c) Sandia Corporation
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
  ----------------------------------------------------------------------------*/
// .NAME vtkSQLDatabase - maintain a connection to an sql database
//
// .SECTION Description
// Abstract base class for all SQL database connection classes.
// Manages a connection to the database, and is responsible for creating
// instances of the associated vtkSQLQuery objects associated with this
// class in order to perform execute queries on the database.
// To allow connections to a new type of database, create both a subclass
// of this class and vtkSQLQuery, and implement the required functions:
//
// Open() - open the database connection, if possible.
// Close() - close the connection.
// GetQueryInstance() - create and return an instance of the vtkSQLQuery
//                      subclass associated with the database type.
//
// The subclass should also provide API to set connection parameters.
//
// This class also provides the function EffectSchema to transform a 
// database schema into a SQL database.
//
// .SECTION Thanks
// Thanks to Andrew Wilson from Sandia National Laboratories for his work
// on the database classes and for the SQLite example. Thanks to David Thompson 
// and Philippe Pebay from Sandia National Laboratories for implementing
// this class.
//
// .SECTION See Also
// vtkSQLQuery
// vtkSQLDatabaseSchema

#ifndef __vtkSQLDatabase_h
#define __vtkSQLDatabase_h

#include "vtkObject.h"

#include "vtkStdString.h" // Because at least one method returns a vtkStdString

class vtkSQLDatabaseSchema;
class vtkSQLQuery;
class vtkStringArray;

// This is a list of features that each database may or may not
// support.  As yet (February 2008) we don't provide access to most of
// them.  
#define VTK_SQL_FEATURE_TRANSACTIONS            1000
#define VTK_SQL_FEATURE_QUERY_SIZE              1001
#define VTK_SQL_FEATURE_BLOB                    1002
#define VTK_SQL_FEATURE_UNICODE                 1003
#define VTK_SQL_FEATURE_PREPARED_QUERIES        1004
#define VTK_SQL_FEATURE_NAMED_PLACEHOLDERS      1005
#define VTK_SQL_FEATURE_POSITIONAL_PLACEHOLDERS 1006
#define VTK_SQL_FEATURE_LAST_INSERT_ID          1007
#define VTK_SQL_FEATURE_BATCH_OPERATIONS        1008
#define VTK_SQL_FEATURE_TRIGGERS                1009

class VTK_IO_EXPORT vtkSQLDatabase : public vtkObject
{
 public:
  vtkTypeRevisionMacro(vtkSQLDatabase, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Open a new connection to the database.
  // You need to set up any database parameters before calling this function.
  // Returns true is the database was opened sucessfully, and false otherwise.
  virtual bool Open() = 0;

  // Description:
  // Close the connection to the database.
  virtual void Close() = 0;
  
  // Description:
  // Return whether the database has an open connection.
  virtual bool IsOpen() = 0;

  // Description:
  // Return an empty query on this database.
  virtual vtkSQLQuery* GetQueryInstance() = 0;
  
  // Description:
  // Did the last operation generate an error
  virtual bool HasError() = 0;
  
  // Description:
  // Get the last error text from the database
  // I'm using const so that people do NOT
  // use the standard vtkGetStringMacro in their
  // implementation, because 99% of the time that
  // will not be the correct thing to do...
  virtual const char* GetLastErrorText() = 0;
  
  // Description:
  // Get the type of the database (e.g. mysql, psql,..).
  virtual char* GetDatabaseType() = 0;
  
  // Description:
  // Get the list of tables from the database.
  virtual vtkStringArray* GetTables() = 0;
    
  // Description:
  // Get the list of fields for a particular table.
  virtual vtkStringArray* GetRecord(const char *table) = 0;

  // Description:
  // Return whether a feature is supported by the database.
  virtual bool IsSupported(int vtkNotUsed(feature)) { return false; }
  
  // Description:
  // Get the URL of the database.
  virtual vtkStdString GetURL() = 0;

  // Description:
  // Return the SQL string with the syntax to create a column inside a
  // "CREATE TABLE" SQL statement.
  // NB: this method implements the following minimally-portable syntax:
  // <column name> <type> <column attributes>
  // It must be overwritten for those SQL backends which have a different 
  // syntax such as, e.g., PostgreSQL.
  virtual vtkStdString GetColumnSpecification( vtkSQLDatabaseSchema* schema,
                                               int tblHandle,
                                               int colHandle );
 
  // Description:
  // For each column type indexed in vtkSQLDatabaseSchema, return the 
  // corresponding SQL string.
  // NB: a minimal set of common SQL types is provided; the backend-specific
  // GetColumnTypeString functions allow for more types.
  virtual vtkStdString GetColumnTypeString( int colType );
 
  // Description:
  // Create a the proper subclass given a URL.
  // The URL format for SQL databases is a true URL of the form:
  //   'protocol://'[[username[':'password]'@']hostname[':'port]]'/'[dbname] .
  static vtkSQLDatabase* CreateFromURL( const char* URL );

  // Description:
  // Effect a database schema.
  virtual bool EffectSchema( vtkSQLDatabaseSchema*, bool dropIfExists = false );

 protected:
  vtkSQLDatabase();
  ~vtkSQLDatabase();

 private:
  vtkSQLDatabase(const vtkSQLDatabase &); // Not implemented.
  void operator=(const vtkSQLDatabase &); // Not implemented.
};

#endif // __vtkSQLDatabase_h
