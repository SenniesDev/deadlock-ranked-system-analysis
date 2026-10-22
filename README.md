# Deadlock Ranked System Analysis

A statistical overview of (Valve's) ranked system implemented into their upcoming game "Deadlock" as of August 2026.

In the attached Jupyter Notebook I run through at the impact of **shields** and **winstreak bonuses**. We can do this by simulating the system under ranging variables with the .cpp file.

This Jupyter Notebook requires you to have the .cpp file on device, if you want to run your own simulations in order to train your own models.

You can go through the Notebook yourself in Google Colab! (Doesn't require you to install any libraries / software on your own PC) 
Getting it working on Google Colab:
<ol>
    <li>Go to Google Colab</li>
    <li>Load the Jupyter Notebook</li>
    <li>Click on the file icon on the left vertical bar</li>
    <li>Wait for the runtime to start</li>
    <li>Click on the upload file icon at the top of the bar</li>
    <li>Find the simulator.cpp file and upload it</li>
</ol>

Some note-worthy mentions:
<ul>
    <li>Eternus is now % based, so anything beyond Eternus 1 is not possible to be accurately determined. In the simulator.cpp I treat Eternus as a normal rank, so just keep that in mind that any rank predictions in that area are very likely not accurate at all.</li>
    <li>The formula I've come up with is quite basic and could definitely be improved, it on average accounts for roughly 70% of the variance in point change, so I've still decided to include it.</li>
    <li>The formula itself is also not valid at a low number of games, it overpredicts in those edge cases. It gradually becomes more accurate, so anything above 10+ games should be valid.</li>
</ul>

And if you've come here just for the summary:
<ul>
    <li>This was quite surprising to me, but the addition of shields and winstreak bonuses brings the break-even winrate from 50% to about 40-41%.</li>
    <li>Winstreaks at a 50% winrate account for roughly 5% of the total points gained, while at 70% account for roughly 10%.</li>
    <li>Each shield on average protects from a loss of around 250-ish points. This is because they are usually triggered right after reaching a new rank.</li>
    <li>Shields in total prevent (in a range of 35-65% winrate) between 15-25% of all potentially lost points, which is a large reason why the break-even rate is so low.</li>
</ul>
