from Globals import *
import Utils2 as uti

grp = xsi.Selection(0)
model = grp.Model
name = grp.Name

guides = [(XSIUtils.DataRepository.GetIdentifier(member, constants.siObjectIdentifier), member)
          for member in grp.Members]

guides.sort()

xsi.DeleteObj(grp)
uti.GroupSetup(model, [guide[1] for guide in guides], name)
