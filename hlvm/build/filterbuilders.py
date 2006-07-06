from SCons.Environment import Environment as Environment
import re,fileinput,os
from string import join as sjoin
from os.path import join as pjoin

def _sedit(input,output,substs):
  res = []
  f=open(output[0].path,'w')
  for subst in substs:
    res.append(re.compile(subst[0],re.MULTILINE|re.DOTALL))
  lines =""
  for line in fileinput.input(input[0].path):
    lines+= line
  count = 0
  for r in res:
    lines = res[count].sub(substs[count][1],lines,0)
    count += 1
  lines += '\n'
  f.write(lines)
  f.close()
  return 0

def QuoteSourceAction(target,source,env):
  if env.Execute(env['with_xmllint'] + ' --relaxng ' + source[0].path + 
      ' test/return0/helloworld.hlx >/dev/null'):
    return 1
  substs = [
    ['<!--.*?-->',''],
    ['<annotation>.*?</annotation>',''],
    ['^[ \\t]*<','<'],
    ['^[ \\t]*([^<])',' \\1'],
    ['[\\\\]','\\\\\\\\'],
    ['"','\\"'],
    ['^([^\\n]*)$','"\\1"'],
    ['^"[ \\t]*?"$\\n','']
  ]
  return _sedit(source,target,substs)

def QuoteSourceMessage(target,source,env):
  return "Converting %s to %s as quoted source" % (source[0],target[0])

def RNGQuoteSource(env):
  a = env.Action(QuoteSourceAction,QuoteSourceMessage)
  b = env.Builder(action=a,suffix='inc',src_suffix='rng',single_source=1)
  env.Append(BUILDERS = {'RNGQuoteSource':b})
  return b

def getSchemaTokens(fname):
  elemPat = re.compile('<element[^>]*name[ ]*=[ ]*"([^"]*)"')
  attrPat = re.compile('<attribute[^>]*name[ ]*=[ ]*"([^"]*)"')
  valuPat = re.compile('<value>\s*([^<\s]*)')
  subsPat = re.compile('[^A-Za-z0-9_]');
  tokens = []
  for line in fileinput.input(fname):
    tokens += elemPat.findall(line)
    tokens += attrPat.findall(line)
    tokens += valuPat.findall(line)
  dict = {}
  for tok in tokens:
    clean_tok = subsPat.sub('_',tok);
    dict[clean_tok] = 1
  result = list(dict.keys())
  result.sort()
  return result

def processFile(preamble,tmplt,output,substs):
  res = []
  for subst in substs:
    res.append(re.compile(subst[0]))
  outf = open(output,"w")
  for infile in [preamble,tmplt]:
    for line in fileinput.input(infile):
      count = 0
      for r in res:
        line = res[count].sub(substs[count][1],line,0)
        count += 1
      outf.write(line)
  outf.close()

def RNGTokenizerAction(target,source,env):
  tgtDir = target[0].dir.path
  ModulePath = re.sub(pjoin(env['BuildDir'],'hlvm','')+'(.*)','\\1',tgtDir)
  Module = re.sub('[/\\\\]','_',ModulePath)
  Schema = re.sub('.*/(.*).rng','\\1',source[0].path)
  PreambleFile   = "utils/tmplt/Preamble_Code";
  HeaderTemplate = "utils/tmplt/Tokenizer_Template.h";
  SourceTemplate = "utils/tmplt/Tokenizer_Template.cpp";     
  HeaderFile = pjoin(tgtDir,Schema) + "Tokenizer.h";
  SourceFile = pjoin(tgtDir,Schema) + "Tokenizer.cpp";
  TokenHashClass = Schema + "TokenHash";
  TokenHashFile = pjoin(tgtDir,TokenHashClass) + ".i";
  tokens = []
  tokens = getSchemaTokens(source[0].path)
  tknFilename = pjoin(tgtDir,Schema) + "Tokens.tmp"
  tknFile = open(tknFilename,"w")
  tknFile.write('struct TokenMap {\n')
  tknFile.write('const char *name; HLVM_'+Module+'::'+Schema+'Tokens token;\n')
  tknFile.write('};\n%%\n')
  for tkn in tokens:
    tknFile.write('"' + tkn + '", HLVM_'+Module+'::TKN_' + tkn + ',\n')
  tknFile.write('%%\n')
  tknFile.close()
  gperfAction = env.Action(
    env['with_gperf'] + " -tcDCIoGl --fast 0 -L C++ -Z " + TokenHashClass +
    " -s 2 -S 1 -k '*' " + tknFilename + " >" + TokenHashFile)
  if env.Execute(gperfAction):
    return 1;
  tokenList = ""
  for tkn in tokens:
    tokenList += "TKN_" + tkn + ",\n    "
  from datetime import date
  substs = [
    ['%SCHEMA_NAME%',Schema],
    ['%AUTHOR%','Generated By HLVM build system'],
    ['%DATE%',date.today().strftime('%c')],
    ['%MODULE%',Module],
    ['%MODULE_PATH%',ModulePath],
    ['%TOKEN_LIST%',tokenList]
  ]
  processFile(PreambleFile,HeaderTemplate,HeaderFile,substs)
  processFile(PreambleFile,SourceTemplate,SourceFile,substs)
  return 0

def RNGTokenizerMessage(target,source,env):
  return "Generating Perfect Hash Tokenizer For " + source[0].path

def RNGTokenizer(env):
  a = env.Action(RNGTokenizerAction,RNGTokenizerMessage)
  b = env.Builder(action=a,suffix='h',src_suffix='rng',single_source=1)
  env.Append(BUILDERS = {'RNGTokenizer':b})
  return 1

def ConfigFileMessage(target,source,env):
  return "Generating Configuration File " + target[0].path

def ConfigFileAction(target,source,env):
  pat = re.compile('@([^@]+)@')
  tgt = open(target[0].path,'w')
  for line in fileinput.input(source[0].path):
    matchobj = pat.search(line)
    if None != matchobj:
      var = matchobj.expand('\\1')
      if env._dict.has_key(var):
        substval = env[var]
        if type(substval) != str:
          substval = `env[var]`
        line = pat.sub(substval,line)
    tgt.write(line)
    continue
  tgt.close()

def ConfigFile(env):
  action  = env.Action(ConfigFileAction,ConfigFileMessage)
  builder = env.Builder(action=action,src_suffix='.in',single_source=1)
  env.Append(BUILDERS = {'ConfigFile':builder})
  return 1
