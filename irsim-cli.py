import sys
import os
import platform

__version__ = "1.0.2"


class IRSyntaxError(Exception):
    pass


class DuplicatedLabelError(Exception):
    pass


class UndefinedLabelError(Exception):
    pass


class DuplicatedVariableError(Exception):
    pass


class CurrentFunctionNoneError(Exception):
    pass


class IRSim():

    def __init__(self):
        super(IRSim, self).__init__()

        self.initialize()

    # Initialize the VM settings
    def initialize(self):
        self.filename = None
        self.ip = -1
        self.entranceIP = -1
        self.pauseRunning = False
        self.offset = 0
        self.instrCnt = 0
        self.codes = list()
        self.mem = None
        self.functionDict = dict()
        self.currentFunction = None
        self.symTable = dict()
        self.labelTable = dict()
        self.callStack = list()
        self.argumentStack = list()
        self.mycodeList = []
        self.updateStatus(None)

    # Load the contents of the file
    def loadFile(self, fname):
        fp = open(fname, 'r')
        lineno = 0
        for line in fp:
            if line.isspace():
                continue
            if self.sanity_check(line, lineno):
                self.mycodeList.append(line.strip().replace('\t', ' '))
                # self.codeList.addItem(line.strip().replace('\t', ' '))
            else:
                break
            lineno += 1
        else:
            self.filename = fname
            self.lineno = lineno
        fp.close()
        if self.entranceIP == -1:
            print(
                'Error', 'Cannot find program entrance. Please make sure the \'main\' function does exist.')
            # QMessageBox.critical(self, 'Error', 'Cannot find program entrance. Please make sure the \'main\' function does exist.')
        if (self.filename is None) or (not self.labelCheck()) or (self.offset > 0x100000) or (self.entranceIP == -1):
            self.updateStatus('Loading failed.')
            self.initialize()
            return

        self.mem = [0] * 262144
        self.updateStatus('File loaded successfully.')

    # Run the codes
    def run(self):
        self.stop()
        self.ip = self.entranceIP
        while True:
            if self.ip < 0 or self.ip >= len(self.codes):
                error_code = 3
                break
            code = self.codes[self.ip]
            # error_code : [0=nextIR; 1=finish; 2=memAccessError; 3=IPError]
            error_code = self.execute_code(code)
            if error_code > 0:
                break
            self.ip += 1
        if error_code == 1:
            # QMessageBox.information(self, 'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
            print(
                'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
            self.updateStatus(
                'Simulation OK. Instruction count = %d' % self.instrCnt)
            return 0
        elif error_code == 2:
            # QMessageBox.critical(self, 'Error', 'An error occurred at line %d: Illegal memory access. \nIf this message keeps popping out, please reload the source file' % (self.ip+1))
            print('Error', 'An error occurred at line %d: Illegal memory access. \nIf this message keeps popping out, please reload the source file' % (self.ip + 1))
            self.updateStatus('Simulation failed: Memory access out of bound.')
            return -2
        elif error_code == 3:
            # QMessageBox.warning(self, 'Warning', 'Program Counter goes out of bound. The running program will be terminated instantly.')
            print('Warning', 'Program Counter goes out of bound. The running program will be terminated instantly.')
            self.updateStatus('Simulation failed: PC error.')
            return -3

        self.ip = -1

    # Stop the running
    def stop(self):
        if self.ip != -1:
            print('clear codeList')
            # self.codeList.item(self.ip).setBackground(self.noBrush)
        self.ip = -1
        self.instrCnt = 0
        self.pauseRunning = False
        # self.watchTable.setRowCount(0)
        self.mem = [0] * 262144
        self.callStack = list()
        self.argumentStack = list()

    # Step by step
    def step(self):
        if self.ip == -1:
            self.stop()
            self.pauseRunning = True
            self.ip = self.entranceIP - 1
        else:
            print('set codeList')
            # self.codeList.item(self.ip).setBackground(self.noBrush)
        self.ip += 1
        if self.ip < 0 or self.ip >= len(self.codes):
            QMessageBox.warning(
                self, 'Warning', 'Program Counter goes out of bound. The running program will be terminated instantly.')
            self.updateStatus('Simulation failed: PC error.')
            self.ip = -1
            self.pauseRunning = False
            return
        code = self.codes[self.ip]
        error_code = self.execute_code(code)
        if error_code == 1:
            # QMessageBox.information(self, 'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
            print(
                'Finish', 'Program has exited gracefully.\nTotal instructions = %d' % self.instrCnt)
            self.updateStatus(
                'Simulation OK. Instruction count = %d' % self.instrCnt)
            self.ip = -1
            self.pauseRunning = False
        elif error_code == 2:
            # QMessageBox.critical(self, 'Error', 'An error occurred at line %d: Illegal memory access' % (self.ip+1))
            print('Error', 'An error occurred at line %d: Illegal memory access' % (
                self.ip + 1))
            self.updateStatus('Simulation failed: Memory access out of bound')
            self.ip = -1
            self.pauseRunning = False
        else:
            print('set codelist s')

    # Update status bar, window title and enable actions
    def updateStatus(self, message):
        if message:
            print(message)
            # self.statusBar().showMessage(message, 5000)

    # Check the syntax of the line and update symbol&label table
    def sanity_check(self, code, lineno):
        strs = code.split()
        relops = ['>', '<', '>=', '<=', '==', '!=']
        arithops = ['+', '-', '*', '/']
        try:
            if strs[0] == 'LABEL' or strs[0] == 'FUNCTION':
                if len(strs) != 3 or strs[2] != ':':
                    raise IRSyntaxError
                if strs[1] in self.labelTable:
                    raise DuplicatedLabelError
                self.labelTable[strs[1]] = lineno
                if strs[1] == 'main':
                    if strs[0] == 'LABEL':
                        raise IRSyntaxError
                    self.entranceIP = lineno
                if strs[0] == 'FUNCTION':
                    self.currentFunction = strs[1]
                    self.functionDict[strs[1]] = list()
                self.codes.append(('LABEL', strs[1]))
            else:
                if self.currentFunction == None:
                    raise CurrentFunctionNoneError
                if strs[0] == 'GOTO':
                    if len(strs) != 2:
                        raise IRSyntaxError
                    self.codes.append(('GOTO', strs[1]))
                elif strs[0] == 'RETURN' or strs[0] == 'READ' or strs[0] == 'WRITE' or strs[0] == 'ARG' or strs[0] == 'PARAM':
                    if len(strs) != 2:
                        raise IRSyntaxError
                    if (strs[0] == 'READ' or strs[0] == 'PARAM') and (not strs[1][0].isalpha()):
                        raise IRSyntaxError
                    self.tableInsert(strs[1])
                    self.codes.append((strs[0], strs[1]))
                elif strs[0] == 'DEC':
                    if len(strs) != 3 or (int(strs[2]) % 4 != 0):
                        raise IRSyntaxError
                    if strs[1] in self.symTable:
                        raise DuplicatedVariableError
                    self.tableInsert(strs[1], int(strs[2]), True)
                    self.codes.append('DEC')
                elif strs[0] == 'IF':
                    if len(strs) != 6 or strs[4] != 'GOTO' or strs[2] not in relops:
                        raise IRSyntaxError
                    self.tableInsert(strs[1])
                    self.tableInsert(strs[3])
                    self.codes.append(
                        ('IF', strs[1], strs[2], strs[3], strs[5]))
                else:
                    if strs[1] != ':=' or len(strs) < 3:
                        raise IRSyntaxError
                    if strs[0][0] == '&' or strs[0][0] == '#':
                        raise IRSyntaxError
                    self.tableInsert(strs[0])
                    if strs[2] == 'CALL':
                        if len(strs) != 4:
                            raise IRSyntaxError
                        self.codes.append(('CALL', strs[0], strs[3]))
                    elif len(strs) == 3:
                        self.tableInsert(strs[2])
                        self.codes.append(('MOV', strs[0], strs[2]))
                    elif len(strs) == 5 and strs[3] in arithops:
                        self.tableInsert(strs[2])
                        self.tableInsert(strs[4])
                        self.codes.append(
                            ('ARITH', strs[0], strs[2], strs[3], strs[4]))
                    else:
                        raise IRSyntaxError
        except (IRSyntaxError, ValueError):
            # QMessageBox.critical(self, 'Error', 'Syntax error at line %d:\n\n%s' % (lineno+1, code))
            print('Error', 'Syntax error at line %d:\n\n%s' %
                  (lineno + 1, code))
            return False
        except DuplicatedLabelError:
            # QMessageBox.critical(self, 'Error', 'Duplicated label %s at line %d:\n\n%s' % (strs[1], lineno+1, code))
            print('Error', 'Duplicated label %s at line %d:\n\n%s' %
                  (strs[1], lineno + 1, code))
            return False
        except DuplicatedVariableError:
            # QMessageBox.critical(self, 'Error', 'Duplicated variable %s at line %d:\n\n%s' % (strs[1], lineno+1, code))
            print('Error', 'Duplicated variable %s at line %d:\n\n%s' %
                  (strs[1], lineno + 1, code))
            return False
        except CurrentFunctionNoneError:
            # QMessageBox.critical(self, 'Error', 'Line %d does not belong to any function:\n\n%s' % (lineno+1, code))
            print('Error', 'Line %d does not belong to any function:\n\n%s' %
                  (lineno + 1, code))
            return False
        return True

    # Check if there is undefined label
    def labelCheck(self):
        try:
            for i in range(self.lineno):
                code = self.mycodeList[i]
                # code = unicode(self.codeList.item(i).text())
                strs = code.split()
                if strs[0] == 'GOTO':
                    if strs[1] not in self.labelTable:
                        raise UndefinedLabelError
                elif strs[0] == 'IF':
                    if strs[5] not in self.labelTable:
                        raise UndefinedLabelError
                elif len(strs) > 2 and strs[2] == 'CALL':
                    if strs[3] not in self.labelTable:
                        raise UndefinedLabelError
        except UndefinedLabelError:
            # QMessageBox.critical(self, 'Error', 'Undefined label at line %d:\n\n%s' % (i+1, code))
            print('Error', 'Undefined label at line %d:\n\n%s' % (i + 1, code))
            return False
        return True

    # Insert variables into symTable
    def tableInsert(self, var, size=4, array=False):
        if var.isdigit():
            raise IRSyntaxError
        if var[0] == '&' or var[0] == '*':
            var = var[1:]
        elif var[0] == '#':
            test = int(var[1:])
            return
        if var in self.symTable:
            return
        self.functionDict[self.currentFunction].append(var)
        if self.currentFunction == 'main':
            self.symTable[var] = self.offset, size, array
            self.offset += size
        else:
            self.symTable[var] = -1, size, array

    # Get Value from a variable
    def getValue(self, var):
        if var[0] == '#':
            return int(var[1:])
        elif var[0] == '&':
            return self.symTable[var[1:]][0]
        elif var[0] == '*':
            return self.mem[self.mem[self.symTable[var[1:]][0] // 4] // 4]
        else:
            return self.mem[self.symTable[var][0] // 4]

    # Executed a single line
    def execute_code(self, code):
        # print 'Excecuting code: ' , code
        self.instrCnt += 1
        try:
            if code[0] == 'READ':
                result = int(input('please input a num: '))
                # result, ok = QInputDialog.getInteger(self, 'IR Simulator - Read', 'Please enter an integral value for %s' % code[1], 0)
                # if ok:
                self.mem[self.symTable[code[1]][0] // 4] = result
            elif code[0] == 'WRITE':
                print('stdout: ', str(self.getValue(code[1])))
                # self.console.append(str(self.getValue(code[1])))
            elif code[0] == 'GOTO':
                self.ip = self.labelTable[code[1]]
            elif code[0] == 'IF':
                value1 = self.getValue(code[1])
                value2 = self.getValue(code[3])
                if eval(str(value1) + code[2] + str(value2)):
                    self.ip = self.labelTable[code[4]]
            elif code[0] == 'MOV':
                value = self.getValue(code[2])
                if code[1][0] == '*':
                    self.mem[self.mem[self.symTable[
                        code[1][1:]][0] // 4] // 4] = value
                else:
                    self.mem[self.symTable[code[1]][0] // 4] = value
            elif code[0] == 'ARITH':
                value1 = self.getValue(code[2])
                value2 = self.getValue(code[4])
                self.mem[self.symTable[code[1]][0] //
                         4] = eval(str(value1) + code[3] + str(value2))
            elif code[0] == 'RETURN':
                if len(self.callStack) == 0:
                    return 1
                else:
                    returnValue = self.getValue(code[1])
                    stackItem = self.callStack.pop()
                    self.ip = stackItem[0]
                    for key in stackItem[2].keys():
                        self.symTable[key] = stackItem[2][key]
                    self.offset = stackItem[3]
                    self.mem[self.symTable[stackItem[1]][0] // 4] = returnValue
            elif code[0] == 'CALL':
                oldAddrs = dict()
                oldOffset = self.offset
                for key in self.functionDict[code[2]]:
                    oldAddrs[key] = self.symTable[key]
                    self.symTable[key] = self.getNewAddr(self.symTable[key][1]), self.symTable[
                        key][1], self.symTable[key][2]
                self.callStack.append((self.ip, code[1], oldAddrs, oldOffset))
                self.ip = self.labelTable[code[2]]
            elif code[0] == 'ARG':
                self.argumentStack.append(self.getValue(code[1]))
            elif code[0] == 'PARAM':
                self.mem[self.symTable[code[1]][0] //
                         4] = self.argumentStack.pop()
            # Not finished yet
        except IndexError:
            return 2
        return 0

    def getNewAddr(self, size):
        ret = self.offset
        self.offset = self.offset + size
        return ret


simulator = IRSim()
simulator.initialize()
simulator.loadFile(sys.argv[1])
print(simulator.run())
