#include "helpers/config.h"

void logTurn(char c) { cestaMapping = cestaMapping + c; }

void calculateOptimalPath() {
    cestaOptim = cestaMapping;
    bool optimizable = true;

    while (optimizable) {
        optimizable = false;
        String newPath = "";

        for (int i = 0; i < cestaOptim.length() - 2; i++) {
            String pattern = cestaOptim.substring(i, i + 3);

            if (pattern == "LBR") {
                newPath += "B";
                i += 2;

                optimizable = true;
            } else if (pattern == "LBS") {
                newPath += "R";
                i += 2;

                optimizable = true;
            } else if (pattern == "RBL") {
                newPath += "B";
                i += 2;

                optimizable = true;
            } else if (pattern == "SBL") {
                newPath += "R";
                i += 2;

                optimizable = true;
            } else if (pattern == "SBS") {
                newPath += "B";
                i += 2;

                optimizable = true;
            } else if (pattern == "LBL") {
                newPath += "S";
                i += 2;

                optimizable = true;
            } else {
                newPath += cestaOptim.charAt(i);
            }
        }

        if (cestaOptim.length() >= 2) {
            newPath += cestaOptim.substring(cestaOptim.length() - 2);
        }

        cestaOptim = newPath;
    }

    cestaOptim.replace("B", "");
}
