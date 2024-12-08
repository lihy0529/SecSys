def get_api_key():
    with open("../../api_key.secret", "r") as file:
        return file.read().strip()

from openai import OpenAI
OpenAIAPI = OpenAI(api_key=get_api_key())

def get_response(messages, max_new_tokens=1000):
    while True:
        try:
            print(messages)
            Response = (
                OpenAIAPI.chat.completions.create(
                    model="gpt-4o-2024-08-06", messages=messages, max_tokens=max_new_tokens
                )
            ).choices[0].message.content
            print("Response: ", Response)
            Response = Response[Response.index("```json") + 7:Response.rindex("```")]
            import json
            
            ReturnValue = json.loads(Response)
            print(ReturnValue)      
            if ReturnValue["status"] == "explain":
                if "explanation" in ReturnValue:
                    return ReturnValue
                else:
                    continue
            if ReturnValue["status"] == "debugging":
                import re
                if "bash_command_1" in ReturnValue and "bash_command_2" in ReturnValue:
                    if re.match(r"^\.\/auto_debug (-)?\d+(.\d+)?$", ReturnValue["bash_command_1"]) and re.match(r"^\.\/auto_debug (-)?\d+(.\d+)?$", ReturnValue["bash_command_2"]):
                        return ReturnValue
                    else:
                        print("Invalid bash command format: ", ReturnValue)
                        continue
        except Exception as e:
            print("Error: ", e)
            messages.append({"role": "user", "content": f"Please make sure your output is in json format!"})
            import traceback
            traceback.print_exc()
            continue

with open("Document.txt", "r") as file:
    Document = file.read()
    
with open("auto_debug.c", "r") as file:
    Code = file.read()
    
SystemPrompt = f"""
You are an expert in C programming. You are given a C file auto_debug.c. The description of the C file is as follows:
{Document}

Please help me debug the C file.

The debugging process should be as follows:
1. write a bash file and run it with two different inputs. The example could be like ./auto_debug 1 and ./auto_debug 10.
2. We will run the bash file and get all the memory traces of the program.
3. Because the memory traces are too long, we will only send you the difference between the expected output and the actual output, as
well as the C file at the corresponding lines.
4. You can continue to test the code with different inputs until you find the bug, up to 5 times.

Example input:
At most 10 different memory traces are sent to you.

 
Your output should be in json format. The first parameter is the status, which could be "debugging" or "explain".
If the status is "debugging", in the second and third parameter, you should give two different bash commands.
If the status is "explain", in the second parameter, you should give the explanation of the bug.

Example output 1:
```json
{{
    "status": "debugging",
    "bash_command_1": "./auto_debug 1",
    "bash_command_2": "./auto_debug 10"
}}
```

Example output 2:
```json
{{
    "status": "explain",
    "explanation": "The bug is at line 10. It fails to initialize the variable i."
}}
```

Make sure your output is in json format. If you cannot identify the bug, you can output an arbitrary command.
Note: keep the command format as ./auto_debug <input>.
"""

def get_prompt(Diff, AllRelatedCode):
    ReturnValue = "Here is the diff:\n" + Diff + "\nHere is the related code:\n" + AllRelatedCode
    return ReturnValue

def auto_debug():
    AllRelatedCode = ""
    Diff = ""
    Messages = [{"role": "system", "content": SystemPrompt}]
    # Add execute permission to the auto_debug binary
    import os
    os.system("chmod +x ./auto_debug")
    
    for i in range(5):

        Response = get_response(Messages)
        if Response["status"] == "explain":
            print(Response["explanation"])
            return
        else:
            print(Response["bash_command_1"])
            print(Response["bash_command_2"])
            Command1 = Response["bash_command_1"]
            Command2 = Response["bash_command_2"]
            
            bash_command = f"""
mkdir -p tmp
{Command1} > tmp/output1.txt
{Command2} > tmp/output2.txt
diff tmp/output1.txt tmp/output2.txt > tmp/diff.txt
            """
            import os
            if not os.path.exists("tmp"):
                os.makedirs("tmp")
            for command in bash_command.split("\n"):
                print(command)
                os.system(command)
            
            with open("tmp/diff.txt", "r") as file:
                Diff = file.read()
            import re
            AllRelatedCode = ""
            LineCount = 0
            for line in Diff.split("\n"):
                LineCount += 1
                if LineCount > 10:
                    break
                Match = re.search(r"\[(\d+)\]", line)
                if Match:
                    Trace = int(Match.group(1))
                    RelatedCode = Code.split("\n")[Trace - 20:Trace + 20]
                    RelatedCode = [f"{Trace - 20 + i}: {line}" for i, line in enumerate(RelatedCode)]
                    AllRelatedCode += "\n".join(RelatedCode) + "---------\n"
                    
            Prompt = get_prompt(Diff, AllRelatedCode)
            Messages.append({"role": "assistant", "content": str(Response)})
            Messages.append({"role": "user", "content": Prompt})
        
    return None

auto_debug()