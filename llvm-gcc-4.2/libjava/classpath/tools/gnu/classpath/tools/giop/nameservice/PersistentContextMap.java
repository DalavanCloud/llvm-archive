/* PersistentContextMap.java -- The persistent context naming map
   Copyright (C) 2006 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


package gnu.classpath.tools.giop.nameservice;

import java.io.File;

import org.omg.CORBA.ORB;
import org.omg.CORBA.Object;

/**
 * The persistent context naming map for the persistent naming service. 
 * 
 * @author Audrius Meskauskas, Lithuania (AudriusA@Bioinformatics.org)
 */
public class PersistentContextMap extends PersistentMap
{
  /**
   * Create the persistent context map that stores information in the given
   * file.
   * 
   * @param an_orb the naming service ORB, used to obtain and produce the object
   *          stringified references.
   * @param mapFile the file, where the persistent information is stored.
   * @param reset if true, the previous naming data are discarded. If false
   *          (normally expected), they are loaded from the persistent memory to
   *          provide the persistence.
   */
  public PersistentContextMap(ORB an_orb, File mapFile, boolean reset)
  {
    super(an_orb, mapFile, reset);
  }

  /**
   * This method expects the PersistentContext as its parameter. The returned
   * description line is the name of the context parent folder.
   */
  protected String object_to_string(Object object)
  {
    PersistentContext pc = (PersistentContext) object;
    return pc.contextFolder.getAbsolutePath();
  }

  /**
   * This method restores the PersistenContext. The description line is
   * interpreted as the folder name, absolute path.
   */
  protected Object string_to_object(String description)
  {
    return new PersistentContext(orb, new File(description), reset);
  }
}
