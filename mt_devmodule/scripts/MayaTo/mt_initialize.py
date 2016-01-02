import pymel.core as pm
import logging
import Renderer as Renderer
import traceback
import sys
import os
import aeNodeTemplates as aet
import mtMenu as mtMenu
import mtShaderTools as shaderTools
import path
import tempfile
import maya.cmds as cmds

reload(Renderer)

log = logging.getLogger("mtLogger")

RENDERER_NAME = "MayaTo"

class MayaToRenderer(Renderer.MayaToRenderer):
    theRendererInstance = None
    @staticmethod
    def theRenderer(arg=None):
        if not MayaToRenderer.theRendererInstance:
            MayaToRenderer.theRendererInstance = MayaToRenderer(RENDERER_NAME , __name__)
            # MayaToRenderer.theRendererInstance = MayaToRenderer(RENDERER_NAME , "mt_initialize")
        return MayaToRenderer.theRendererInstance
    
    def __init__(self, rendererName, moduleName):
        Renderer.MayaToRenderer.__init__(self, rendererName, moduleName)
        self.rendererTabUiDict = {}
        self.rendererMenu = None
        
    def createRendererMenu(self):
        self.rendererMenu = mtMenu.MtMenu()

    def removeRendererMenu(self):
        if self.rendererMenu is not None:
            pm.deleteUI(self.rendererMenu)
        self.rendererMenu = None
            
    def getEnumList(self, attr):
        return [(i, v) for i, v in enumerate(attr.getEnums().keys())]

    def updateTest(self, dummy=None):
        print "UpdateTest", dummy             

    def addUserTabs(self):
        pm.renderer(self.rendererName, edit=True, addGlobalsTab=self.renderTabMelProcedure("AOVs"))    
        pm.renderer(self.rendererName, edit=True, addGlobalsTab=self.renderTabMelProcedure("Environment"))    
        
    def updateEnvironment(self):
        envDict = self.rendererTabUiDict['environment']
        envType = self.renderGlobalsNode.environmentType.get()

    def MayaToEnvironmentCreateTab(self):
        log.debug("MayaToEnvironmentCreateTab()")
        self.createGlobalsNode()
        parentForm = pm.setParent(query=True)
        pm.setUITemplate("attributeEditorTemplate", pushTemplate=True)
        scLo = self.rendererName + "AOScrollLayout"
        envDict = {}
        self.rendererTabUiDict['environment'] = envDict
        uiDict = envDict
        with pm.scrollLayout(scLo, horizontalScrollBarThickness=0):
            with pm.columnLayout(self.rendererName + "ColumnLayout", adjustableColumn=True, width=400):
                with pm.frameLayout(label="Environment Lighting", collapsable=False):
                    pass
                
        pm.setUITemplate("attributeEditorTemplate", popTemplate=True)
        pm.formLayout(parentForm, edit=True, attachForm=[ (scLo, "top", 0), (scLo, "bottom", 0), (scLo, "left", 0), (scLo, "right", 0) ])
        pm.scriptJob(attributeChange=[self.renderGlobalsNode.environmentType, pm.Callback(self.uiCallback, tab="environment")])        
        pm.scriptJob(attributeChange=[self.renderGlobalsNode.skyModel, pm.Callback(self.uiCallback, tab="environment")])   
        pm.scriptJob(attributeChange=[self.renderGlobalsNode.physicalSun, pm.Callback(self.uiCallback, tab="environment")])           
        self.updateEnvironment()     

    def MayaToEnvironmentUpdateTab(self):
        log.debug("MayaToEnvironmentUpdateTab()")
        self.updateEnvironment()

    def MayaToAOVSelectCommand(self, whichField):
        log.debug("MayaTo")
        aovDict = self.rendererTabUiDict['aovs']
        if whichField == "source":
            pm.button(aovDict['aovButton'], edit=True, enable=True, label="Add selected Shaders")
            pm.textScrollList(aovDict['aovDestField'], edit=True, deselectAll=True) 
        if whichField == "dest":
            pm.button(aovDict['aovButton'], edit=True, enable=True, label="Remove selected Shaders")
            pm.textScrollList(aovDict['aovSourceField'], edit=True, deselectAll=True) 

    def MayaToAOVUpdateDestList(self):
        aovDict = self.rendererTabUiDict['aovs']
        pm.textScrollList(aovDict['aovDestField'], edit=True, removeAll=True) 
        aovList = self.MayaToGetAOVConnections()
        pm.textScrollList(aovDict['aovDestField'], edit=True, append=aovList)
        
    def MayaToAOVButtonCommand(self, args=None):
        log.debug("MayaToAOVButtonCommand " + str(args))
        aovDict = self.rendererTabUiDict['aovs']
        label = pm.button(aovDict['aovButton'], query=True, label=True)
        if "Add selected Shaders" in label:
            log.debug("MayaToAOVButtonCommand: adding selected shaders")
            selectedItems = pm.textScrollList(aovDict['aovSourceField'], query=True, selectItem=True) 
            for item in selectedItems:
                log.debug("Adding " + item)
                node = pm.createNode(item)
                indices = self.renderGlobalsNode.AOVs.getArrayIndices()
                index = 0
                if len(indices) > 0:
                    index = indices[-1] + 1
                node.message >> self.renderGlobalsNode.AOVs[index]
            self.MayaToAOVUpdateDestList()
            
        if "Remove selected Shaders" in label:
            log.debug("MayaToAOVButtonCommand: removing selected shaders")
            selectedItems = pm.textScrollList(aovDict['aovDestField'], query=True, selectItem=True) 
            for item in selectedItems:
                shaderName = item.split(" ")[0]
                log.debug("Removing " + shaderName)
                shader = pm.PyNode(shaderName)
                attribute = shader.message.outputs(p=True)[0]
                pm.delete(shader)
                attribute.remove()
            self.MayaToAOVUpdateDestList()
            
    def MayaToDoubleClickCommand(self):
        log.debug("MayaToDoubleClickCommand")
        aovDict = self.rendererTabUiDict['aovs']
        selectedItems = pm.textScrollList(aovDict['aovDestField'], query=True, selectItem=True) 
        pm.select(selectedItems[0].split(" ")[0])
            
    def MayaToGetAOVConnections(self):
        aoList = self.renderGlobalsNode.AOVs.inputs()
        aovList = []
        for aov in aoList:
            aovList.append(aov.name() + " (" + str(aov.type()) + ")")
        return aovList
            
    def MayaToAOVsCreateTab(self):
        log.debug("MayaToAOVsCreateTab()")
        aovDict = {}
        self.rendererTabUiDict['aovs'] = aovDict
        self.createGlobalsNode()
        parentForm = pm.setParent(query=True)
        pm.setUITemplate("attributeEditorTemplate", pushTemplate=True)
        scLo = self.rendererName + "AOScrollLayout"
        with pm.scrollLayout(scLo, horizontalScrollBarThickness=0):
            with pm.columnLayout("ColumnLayout", adjustableColumn=True, width=400):
                with pm.frameLayout(label="AOVs", collapsable=True, collapse=False):
                    with pm.columnLayout():
                        with pm.paneLayout(configuration="vertical2", paneSize=(1, 25, 100)):
                            aovDict['aovSourceField'] = pm.textScrollList("AOVSource", ams=True, append=self.aovShaders, selectCommand=pm.Callback(self.MayaToAOVSelectCommand, "source"))
                            aovList = self.MayaToGetAOVConnections()
                            aovDict['aovDestField'] = pm.textScrollList("AOVDest", append=aovList, ams=True, dcc=self.MayaToDoubleClickCommand, selectCommand=pm.Callback(self.MayaToAOVSelectCommand, "dest"))
                        aovDict['aovButton'] = pm.button(label="Selection", enable=False, c=self.MayaToAOVButtonCommand)
                        
        pm.setUITemplate("attributeEditorTemplate", popTemplate=True)
        pm.formLayout(parentForm, edit=True, attachForm=[ (scLo, "top", 0), (scLo, "bottom", 0), (scLo, "left", 0), (scLo, "right", 0) ])
                    

    def MayaToAOVsUpdateTab(self):
        log.debug("MayaToAOVsUpdateTab()")
            
    def MayaToCommonGlobalsCreateTab(self):
        self.OpenMayaCommonGlobalsCreateTab()            

    def MayaToCommonGlobalsUpdateTab(self):
        self.OpenMayaCommonGlobalsUpdateTab()    
                    
    def MayaToRendererCreateTab(self):
        log.debug("MayaToRendererCreateTab()")
        self.createGlobalsNode()
        parentForm = pm.setParent(query=True)
        pm.setUITemplate("renderGlobalsTemplate", pushTemplate=True)
        pm.setUITemplate("attributeEditorTemplate", pushTemplate=True)
        scLo = self.rendererName + "ScrollLayout"
        if self.rendererTabUiDict.has_key('common'):
            self.rendererTabUiDict.pop('common')        
        uiDict = {}
        self.rendererTabUiDict['common'] = uiDict

        
        with pm.scrollLayout(scLo, horizontalScrollBarThickness=0):
            with pm.columnLayout(self.rendererName + "ColumnLayout", adjustableColumn=True, width=400):
                with pm.frameLayout(label="Filtering", collapsable=True, collapse=False):
                    with pm.columnLayout(self.rendererName + "ColumnLayout", adjustableColumn=True, width=400):
                        attr = pm.Attribute(self.renderGlobalsNodeName + ".filtertype")
                        
                with pm.frameLayout(label="Features", collapsable=True, collapse=False):
                    with pm.columnLayout(self.rendererName + "ColumnLayout", adjustableColumn=True, width=400):
                        self.addRenderGlobalsUIElement(attName='doMotionBlur', uiType='bool', displayName='Motion Blur', default=False, uiDict=uiDict)
                        self.addRenderGlobalsUIElement(attName='doDof', uiType='bool', displayName='Depth Of Field', default=False, uiDict=uiDict)
                                        
        pm.setUITemplate("renderGlobalsTemplate", popTemplate=True)                    
        pm.setUITemplate("attributeEditorTemplate", popTemplate=True)
        pm.formLayout(parentForm, edit=True, attachForm=[ (scLo, "top", 0), (scLo, "bottom", 0), (scLo, "left", 0), (scLo, "right", 0) ])
        self.MayaToRendererUpdateTab()

    def MayaToRendererUpdateTab(self, dummy=None):
        self.createGlobalsNode()
        log.debug("MayaToRendererUpdateTab()")
        if not self.rendererTabUiDict.has_key('common'):
            return
        uiDict = self.rendererTabUiDict['common']
            
                    
    def xmlFileBrowse(self, args=None):
        print "xmlfile", args
        filename = pm.fileDialog2(fileMode=0, caption="XML Export File Name")
        if len(filename) > 0:
            print "Got filename", filename
            self.rendererTabUiDict['xml']['xmlFile'].setText(filename[0])
    
    def dirBrowse(self, args=None):
        dirname = pm.fileDialog2(fileMode=3, caption="Select dir")
        if len(dirname) > 0:
            self.rendererTabUiDict['opti']['optiField'].setText(dirname[0])
        
    def MayaToTranslatorCreateTab(self):
        log.debug("MayaToTranslatorCreateTab()")
        self.createGlobalsNode()
        parentForm = pm.setParent(query=True)
        pm.setUITemplate("attributeEditorTemplate", pushTemplate=True)
        scLo = self.rendererName + "TrScrollLayout"
        
        with pm.scrollLayout(scLo, horizontalScrollBarThickness=0):
            with pm.columnLayout(self.rendererName + "TrColumnLayout", adjustableColumn=True, width=400):
                with pm.frameLayout(label="Translator", collapsable=True, collapse=False):
                    attr = pm.Attribute(self.renderGlobalsNodeName + ".translatorVerbosity")
                    ui = pm.attrEnumOptionMenuGrp(label="Translator Verbosity", at=self.renderGlobalsNodeName + ".translatorVerbosity", ei=self.getEnumList(attr))                                         
                with pm.frameLayout(label="Additional Settings", collapsable=True, collapse=False):
                    ui = pm.floatFieldGrp(label="Scene scale:", value1=1.0, numberOfFields=1)
                    pm.connectControl(ui, self.renderGlobalsNodeName + ".sceneScale", index=2)
                         
        pm.setUITemplate("attributeEditorTemplate", popTemplate=True)
        pm.formLayout(parentForm, edit=True, attachForm=[ (scLo, "top", 0), (scLo, "bottom", 0), (scLo, "left", 0), (scLo, "right", 0) ])

    def MayaToTranslatorUpdateTab(self):
        log.debug("MayaToTranslatorUpdateTab()")

    def uiCallback(self, **args):
        log.debug("uiCallback()")
        
        if args['tab'] == "environment":
            self.updateEnvironment()
        
    def createImageFormats(self):
        if self.renderGlobalsNode:
            iList = self.renderGlobalsNode.imageFormat.getEnums()
            self.imageFormats = []
            self.imageFormats.extend(iList)

    def registerNodeExtensions(self):
        """Register MayaTo specific node extensions. e.g. camera type, diaphram_blades and others
        """
        pass
        # we will have a thinlens camera only
        # pm.addExtension(nodeType="camera", longName="mt_cameraType", attributeType="enum", enumName="Pinhole:Thinlens", defaultValue = 0)
            
    def removeLogFile(self):
        logfile = pm.workspace.path + "/applelog.log"
        try:
            if os.path.exists(logfile):
                os.remove(logfile)
        except:
            pass

    # this method only exists because I was unable to write the logfile during rendering
    # it will be called from the plugin
    def showLogFile(self, logfile):
        log.debug("Trying to open logfile: {0}".format(logfile))
        if os.path.exists(logfile):
            lh = open(logfile, 'r')
            rl = lh.readlines()
            for l in rl:
                sys.__stdout__.write(l)
        else:
            print "Logfile", logfile, "not found"
            
    def renderProcedure(self, width, height, doShadows, doGlow, camera, options):
        log.debug("renderProcedure")
        self.removeLogFile()
        print "renderProcedure", width, height, doShadows, doGlow, camera, options
        self.createGlobalsNode()    
        self.preRenderProcedure()
        self.setImageName()
        
        if pm.about(batch=True):
            pm.mayatoMayaTo()
        else:
            pm.mayatoMayaTo(width=width, height=height, camera=camera)
            
        
    def startIprRenderProcedure(self, editor, resolutionX, resolutionY, camera):
        self.ipr_isrunning = True
        log.debug("startIprRenderProcedure")
        print "startIprRenderProcedure", editor, resolutionX, resolutionY, camera
        self.createGlobalsNode()    
        self.preRenderProcedure()
        self.setImageName()
        pm.mayatoMayaTo(width=resolutionX, height=resolutionY, camera=camera, startIpr=True)
        self.postRenderProcedure()
        
    def stopIprRenderProcedure(self):
        self.ipr_isrunning = False
        log.debug("stopIprRenderProcedure")
        pm.mayatoMayaTo(stopIpr=True)
        self.postRenderProcedure()
            
    def preRenderProcedure(self):
        self.createGlobalsNode()
        if self.renderGlobalsNode.threads.get() == 0:
            # TODO this is windows only, search for another solution...
            numThreads = int(os.environ['NUMBER_OF_PROCESSORS'])
            self.renderGlobalsNode.threads.set(numThreads)
            # lets reduce the number of threads to stay as interactive as possible
            if self.ipr_isrunning:
                if numThreads > 4:
                    numThreads = numThreads - 1
            
        if self.renderGlobalsNode.useOptimizedTextures.get():
            if not self.renderGlobalsNode.optimizedTexturePath.get() or len(self.renderGlobalsNode.optimizedTexturePath.get()) == 0:
                try:
                    optimizedPath = pm.workspace.path / pm.workspace.fileRules['renderData'] / "optimizedTextures"
                except:
                    optimizedPath = path.path(tempfile.gettempdir()) / "optimizedTextures"
                if not os.path.exists(optimizedPath):
                    optimizedPath.makedirs()
                self.renderGlobalsNode.optimizedTexturePath.set(str(optimizedPath))
                    
    def postRenderProcedure(self):
        shaderTools.removeAutoShaderNodes()

    def afterGlobalsNodeReplacement(self):
        log.debug("afterGlobalsNodeReplacement")        
        self.rendererTabUiDict = {}

    def aeTemplateCallback(self, nodeName):
        log.debug("aeTemplateCallback: " + nodeName)
        aet.AEmtNodeTemplate(nodeName)
        
        
    def connectNodeToNodeOverrideCallback(self, srcNode, destNode):
        log.debug("connectNodeToNodeOverrideCallback {0} {1}".format(srcNode, destNode))
        dn = pm.PyNode(destNode)
        sn = pm.PyNode(srcNode)
#         if dn.type() in ["CoronaSurface"]:
#             if sn.hasAttr("outColor"):
#                 sn.outColor >> dn.diffuse
#                 return 0
#             if sn.type() == "CoronaRoundCorners":
#                 sn.outNormal >> dn.roundCornersMap
#                 return 0
        return 1
    
    def createRenderNode(self, nodeType=None, postCommand=None):
        print "createRenderNode nodeType ", nodeType, "postCommand", postCommand
        log.debug("createRenderNode callback for renderer {0} with node: {1}".format(self.rendererName.lower(), nodeType))
        nodeClass = None
        rendererName = self.rendererName.lower()
        for cl in pm.getClassification(nodeType):
            if (rendererName+"/material") in cl.lower():
                nodeClass = "shader"
            if (rendererName + "/texture") in cl.lower():
                nodeClass = "texture"
                
        if nodeClass == "shader":
            mat = pm.shadingNode(nodeType, asShader=True)
            shadingGroup = pm.sets(renderable=True, noSurfaceShader=True, empty=True, name="{0}SG".format(mat))
            mat.outColor >> shadingGroup.surfaceShader
        else:
            mat = pm.shadingNode(nodeType, asTexture=True)
            
        if postCommand is not None:
            postCommand = postCommand.replace("%node", str(mat))
            postCommand = postCommand.replace("%type", '\"\"')
            pm.mel.eval(postCommand)
        return ""
        

"""
This procedure loads all AETemplates that are loaceted in the AETemplates module. 
Normally if you load pymel, it automatically loads the templates but only the ones it finds in the
very first AETemplates directory. If you have several OpenMaya renderers loaded or if you have your own
AETemplates directory, the automatic loading will not work. So I replace it with this procedure.
"""

def loadAETemplates():    
    rendererName = "MayaTo"
    aeDir = path.path(__file__).dirname() + "/" + rendererName + "/AETemplate/"
    for d in aeDir.listdir("*.py"):
        if d.endswith("Template.py"):
            templateName = d.basename().replace(".py", "")
            pythonCommand = "import {1}.AETemplate.{0}".format(templateName, rendererName)
            melCommand = 'python("{0}");'.format(pythonCommand)
            # log.debug("load aeTemplate: " + templateName + " : " + melCommand)
            pm.mel.eval(melCommand)

def loadPlugins():
    plugins = ["LoadShadersPlugin"]
    for plugin in plugins:
        try:
            log.debug("Loading additional plugin: {0}".format(plugin))
            if not pm.pluginInfo(plugin, query=True, loaded=True):
                pm.loadPlugin(plugin)      
        except:
            traceback.print_exc(file=sys.__stderr__)
            log.error("Loading of additional plugin: {0} failed.".format(plugin))    
        
def theRenderer():
    return MayaToRenderer.theRenderer()
        
def initRenderer():
    try:
        log.debug("Init renderer MayaTo")
        theRenderer().registerRenderer()
        if not pm.about(batch=True):
            loadAETemplates()
            theRenderer().createRendererMenu()
        loadPlugins()
    except:
        traceback.print_exc(file=sys.__stderr__)
        log.error("Init renderer MayaTo FAILED")
        
def unregister():
    theRenderer().removeRendererMenu()
    theRenderer().unRegisterRenderer()
    log.debug("Unregister done")

def uiCallback(*args):
    theRenderer().uiCallback(args)

