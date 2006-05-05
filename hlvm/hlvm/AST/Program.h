//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Program.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Program
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_PROGRAM_H
#define HLVM_AST_PROGRAM_H

#include <hlvm/AST/Function.h>

namespace hlvm {
namespace AST {

  class Block; // Forward declare
  class SignatureType;  // Forward declare

  /// This class represents a Program in the HLVM Abstract Syntax Tree.  
  /// A Program is a function with a specific set of arguments. It represents
  /// a starting point for any execution. To be executable, a Bundle must have
  /// at least one Program node in it. The Program node is simply introduced
  /// to ensure the signature of the function is correct and to serve as a way
  /// to identify Program's quickly.
  /// @brief HLVM AST Function Node
  class Program : public Function
  {
    /// @name Constructors
    /// @{
    public:
      Program(
        Node* parent, ///< The bundle in which the function is defined
        const std::string& name ///< The name of the function
      ) : Function(SignatureTy,parent,name,ProgramID) {}
      virtual ~Program();

    /// @}
    /// @name Accessors
    /// @{
    public:
      static inline bool classof(const Program*) { return true; }
      static inline bool classof(const Node* N) { return N->isProgram(); }

    /// @}
    /// @name Data
    /// @{
    private:
      static SignatureType* SignatureTy; ///< The signature for programs
      static SignatureType* initSignature(); 
    /// @}
  };
} // AST
} // hlvm
#endif
